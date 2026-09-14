/**
 * 
 * Copyright 2026 Juha Ollila
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#include <stdio.h>
#include <stdbool.h>

#include "cpu.h"

u8 mem[MEM_SIZE];



void init_cpu(CPU_CTX *cpu)
{
	cpu->ac = 0;
	cpu->xr = 0;
	cpu->yr = 0;
	cpu->sp = 0xff;
	cpu->sr = 0;
	cpu->pc = 0x0;
	cpu->halt = false;
}

void init_memory(void)
{
	for(int i = 0; i < MEM_SIZE;) {
		mem[i++] = 0x0;
		// mem[i++] = 0x0f;
		// mem[i++] = 0xf0;
		// mem[i++] = 0xff;
	}
	/* Install default BRK vector (CBINV), C64 specific */
	mem[0x316] = 0x66;
	mem[0x317] = 0xfe;
}

static u8 fetch_byte(CPU_CTX *cpu)
{
	return mem[cpu->pc++];
}

static u8 peek_byte(CPU_CTX *cpu)
{
	return mem[cpu->pc];
}

static u16 fetch_word(CPU_CTX *cpu)
{
	return (mem[cpu->pc++] + 256 * mem[cpu->pc++]);
}

static u16 peek_word(CPU_CTX *cpu)
{
	return (mem[cpu->pc] + 256 * mem[cpu->pc+1]);
}

static void push_byte(CPU_CTX *cpu, u8 i)
{
	mem[0x100 + cpu->sp] = i;
	cpu->sp--;
}

static void push_word(CPU_CTX *cpu, u16 i)
{
	mem[0x100 + cpu->sp] = ((i >> 8) & 0xff);
	cpu->sp--;
	mem[0x100 + cpu->sp] = (i & 0xff);
	cpu->sp--;
}

static u8 pull_byte(CPU_CTX *cpu)
{
	cpu->sp++;
	return mem[0x100 + cpu->sp];
}

static u16 pull_word(CPU_CTX *cpu)
{
	u16 value;
	cpu->sp++;
	value = mem[0x100 + cpu->sp];
	cpu->sp++;
	value = value + (0x100 * mem[0x100 + cpu->sp]);
	// printf("pull_word: %04x\n", value);
	return value;	
}

static void chrout(u8 i)
{
	if (i == 0xd) i = '\n';
	if (i >= 0x60) i = '.'; 
	putchar(i);
}

void set_nz_flags(CPU_CTX *cpu, u8 i)
{
	if (i == 0) cpu->sr |= zero_flag; else cpu->sr &= (~zero_flag);
	if (i >= 0x80) cpu->sr |= negative_flag; else cpu->sr &= (~negative_flag);
}

void set_cmp_flags(CPU_CTX *cpu, u8 i, u8 j)
{
	i8 result;
	if (i < j) {
		cpu->sr &= (~zero_flag);
		cpu->sr &= (~carry_flag);
		result = (i8)i - (i8)j;
		if ((result & 0x80) == 0x80) cpu->sr |= negative_flag; else cpu->sr &= (~negative_flag); 
	} else if(i > j) {
		cpu->sr &= (~zero_flag);
		cpu->sr |= carry_flag;
		result = (i8)i - (i8)j;
		if ((result & 0x80) == 0x80) cpu->sr |= negative_flag; else cpu->sr &= (~negative_flag); 
	} else {
		cpu->sr |= zero_flag;
		cpu->sr |= carry_flag;
		cpu->sr &= (~negative_flag);
	}
}

u8 adc(CPU_CTX *cpu, u8 i, u8 j)
{
	u8 carry = (cpu->sr & carry_flag) ? 1 : 0;

	/* calculate binary sum */
	int sum = i + j + carry; 
	u8 r = (u8)sum;

	/* binary mode flags */
	if (sum > 0xff) {
		cpu->sr |= carry_flag;
	} else {
		cpu->sr &= (~carry_flag);
	}
	if (((i ^ r) & (j ^ r) & 0x80) != 0) {
		cpu->sr |= overflow_flag;
	} else {
		cpu->sr &= (~overflow_flag);
	}
	set_nz_flags(cpu, r);

	/* decimal mode */
	if (cpu->sr & decimal_flag) {
		if ((i & 0xf) + (j & 0xf) + carry > 9) sum = sum + 0x6;
		if (sum > 0x99) sum = sum + 0x60;
		r = (u8) sum;
		/* decimal mode affects only to carry flag */
		if (sum > 0xff) {
			cpu->sr |= carry_flag;
		} else {
			cpu->sr &= (~carry_flag);
		}
	}

	return r;
}

u8 sbc(CPU_CTX *cpu, u8 i, u8 j)
{
	int borrow = (int)((cpu->sr & carry_flag) ? 0 : 1);
	int diff = (int)i - (int) j - borrow;

	u8 r = (u8)diff;

	/* binary mode flags */
	if ((((i ^ j) & (i ^ r)) & 0x80) != 0) {
		cpu->sr |= overflow_flag;
	} else {
		cpu->sr &= (~overflow_flag);
	}
	set_nz_flags(cpu, r);

	if (cpu->sr & decimal_flag) {
		if (((i & 0xf) - borrow) < (j & 0xf)) diff = diff - 0x6;
		if (diff < 0) diff = diff - 0x60;
		r = (u8) diff;
	}

	/* set carry flag if needed */
	if (diff >= 0x0) {
		cpu->sr |= carry_flag; /* no borrow */
	} else {
		cpu->sr &= (~carry_flag); /* borrow */
	}

	return r;
}

static u8 asl(CPU_CTX *cpu, u8 i)
{
	if ((i & 0x80) == 0x80) {
		cpu->sr |= carry_flag;
	} else {
		cpu->sr &= (~carry_flag);
	}
	u8 r = i << 1;
	set_nz_flags(cpu, r);
	return r;
}

static u8 rol(CPU_CTX *cpu, u8 i)
{	u8 r = i << 1;
	if ((cpu->sr & carry_flag) == carry_flag) {
		r |= 0x1;
	}
	if ((i & 0x80) == 0x80) {
		cpu->sr |= carry_flag;
	} else {
		cpu->sr &= (~carry_flag);
	}
	set_nz_flags(cpu, r);
	return r;
}

static u8 lsr(CPU_CTX *cpu, u8 i)
{	u8 r = i >> 1;
	if ((i & 0x1) == 0x1) {
		cpu->sr |= carry_flag;
	} else {
		cpu->sr &= (~carry_flag);
	}
	set_nz_flags(cpu, r);
	return r;
}

static u8 ror(CPU_CTX *cpu, u8 i)
{	u8 r = i >> 1;
	if ((cpu->sr & carry_flag) == carry_flag) {
		r |= 0x80;
	}
	if ((i & 0x1) == 0x1) {
		cpu->sr |= carry_flag;
	} else {
		cpu->sr &= (~carry_flag);
	}
	set_nz_flags(cpu, r);
	return r;
}

static void bit(CPU_CTX *cpu, u8 i)
{
	if ((cpu->ac & i) == 0) cpu->sr |= zero_flag; else cpu->sr &= (~zero_flag);
	if (i & 0x80) cpu->sr |= negative_flag; else cpu->sr &= (~negative_flag);
	if (i & 0x40) cpu->sr |= overflow_flag; else cpu->sr &= (~overflow_flag);
}

static void print_registers(CPU_CTX *cpu)
{
	printf("   PC   SR AC XR YR SP\n");
	printf("; %04x %02x %02x %02x %02x %02x\n", cpu->pc, cpu->sr, cpu->ac, cpu->xr, cpu->yr, cpu->sp);
}

void execute_instruction(CPU_CTX *cpu)
{
	u8 instruction, zp;
	i8 offset;
	u16 address;

	if (cpu->pc == 0xffd2) {
		chrout(cpu->ac);
		instruction = 0x60; /* RTS */
	} else {
		instruction = fetch_byte(cpu);
	}

	// printf("pc: %04x instruction: %02x\n", cpu->pc-1, instruction);
	switch (instruction) {
	case 0x0: /* BRK */
		/**
		 *
		 * What happens when BRK instruction is executed:
		 *
		 * CPU saves PC and SR to stack and jumps to IRQ routine.
		 * 
		 * Kernal's IRQ routine saves AC, XR and YR to stack and checks
		 * B flag in SR. After that it jumps via CBINV or CINV vector
		 * in RAM. CBINV vector is used if B (break) flag is set in SR.
		 *
		 * CBINV = $316-$317, this vector points to $fe66 in ROM
		 * ROM routine calls RESTOR, IOINIT and CINT Kernal routines
		 * and jumps to basic warm start.
		 *
		 */

		/* set break flag */
		cpu->sr |= break_flag;

		/* push PC and SR to stack (done by CPU) */
		push_word(cpu, cpu->pc);
		push_byte(cpu, cpu->sr);

		/* push AC, XR and YR to stack (done by IRQ routine) */
		push_byte(cpu, cpu->ac);
		push_byte(cpu, cpu->xr);
		push_byte(cpu, cpu->yr);

		address = mem[0x316] + (0x100 * mem[0x317]);
		/* if default CBINV vector then halt execution else jump via CBINV */
		if (address == 0xfe66) {
			printf("break\n\n");
			cpu->halt = true;
		} else {
			cpu->pc = address;
		}
		break;
	case 0x1: /* ORA (zp,x) */
		zp = fetch_byte(cpu);
		address = mem[zp+cpu->xr] + (0x100 * mem[zp + cpu->xr + 1]);
		cpu->ac = cpu->ac | mem[address];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x5: /* ORA zp */
		cpu->ac = cpu->ac | mem[fetch_byte(cpu)];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x6: /* ASL zp */
		zp = fetch_byte(cpu);
		mem[zp] = asl(cpu, mem[zp]);
		break;
	case 0x8: /* PHP */
		push_byte(cpu, (cpu->sr | one_flag | break_flag));
		break;
	case 0x9: /* ORA # */
		cpu->ac = cpu->ac | fetch_byte(cpu);
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0xa: /* ASL */
		cpu->ac = asl(cpu, cpu->ac);
		break;
	case 0xd: /* ORA abs */
		cpu->ac = cpu->ac | mem[fetch_word(cpu)];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0xe: /* ASL abs */
		address = fetch_word(cpu);
		mem[address] = asl(cpu, mem[address]);
		break;
	case 0x10: /* BPL rel */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & negative_flag) != negative_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0x11: /* ORA (zp),y */
		zp = fetch_byte(cpu);
		address = mem[zp] + (0x100 * mem[zp + 1]) + cpu->yr;
		cpu->ac = cpu->ac | mem[address];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x15: /* ORA zp,x */
		cpu->ac = cpu->ac | mem[fetch_byte(cpu) + cpu->xr];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x16: /* ASL zp,x */
		zp = fetch_byte(cpu);
		mem[zp + cpu->xr] = asl(cpu, mem[zp + cpu->xr]);
		break;
	case 0x18: /* CLC */
		cpu->sr &= (~carry_flag);
		break;
	case 0x19: /* ORA abs,y */
		cpu->ac = cpu->ac | mem[fetch_word(cpu) + cpu->yr];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x1d: /* ORA abs,x */
		cpu->ac = cpu->ac | mem[fetch_word(cpu) + cpu->xr];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x1e: /* ASL abs,x */
		address = fetch_word(cpu) + cpu->xr;
		mem[address] = asl(cpu, mem[address]);
		break;
	case 0x20: /* JSR abs */
		push_word(cpu, cpu->pc + 1);
		cpu->pc = fetch_word(cpu);
		// printf("jsr %04x\n", cpu->pc);
		break;
	case 0x21: /* AND (zp,x) */
		zp = fetch_byte(cpu);
		address = mem[zp+cpu->xr] + (0x100 * mem[zp + cpu->xr + 1]);
		cpu->ac = cpu->ac & mem[address];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x24: /* BIT zp */
		bit(cpu, mem[fetch_byte(cpu)]);
		break;
	case 0x25: /* AND zp */
		cpu->ac = cpu->ac & mem[fetch_byte(cpu)];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x26: /* ROL zp */
		zp = fetch_byte(cpu);
		mem[zp] = rol(cpu, mem[zp]);
		break;
	case 0x28: /* PLP */
		cpu->sr = pull_byte(cpu);
		break;
	case 0x29: /* AND # */
		cpu->ac = cpu->ac & fetch_byte(cpu);
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x2a: /* ROL */
		cpu->ac = rol(cpu, cpu->ac);
		break;
	case 0x2c: /* BIT abs */
		bit(cpu, mem[fetch_word(cpu)]);
		break;
	case 0x2d: /* AND abs */
		cpu->ac = cpu->ac & mem[fetch_word(cpu)];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x2e: /* ROL abs */
		address = fetch_word(cpu);
		mem[address] = rol(cpu, mem[address]);
		break;
	case 0x30: /* BMI rel */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & negative_flag) == negative_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0x31: /* AND (zp),y */
		zp = fetch_byte(cpu);
		address = mem[zp] + (0x100 * mem[zp + 1]) + cpu->yr;
		cpu->ac = cpu->ac & mem[address];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x35: /* AND zp,x */
		cpu->ac = cpu->ac & mem[fetch_byte(cpu) + cpu->xr];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x36: /* ROL zp,x */
		zp = fetch_byte(cpu);
		mem[zp + cpu->xr] = rol(cpu, mem[zp + cpu->xr]);
		break;
	case 0x38: /* SEC */
		cpu->sr |= carry_flag;
		break;
	case 0x39: /* AND abs,y */
		cpu->ac = cpu->ac & mem[fetch_word(cpu) + cpu->yr];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x3d: /* AND abs,x */
		cpu->ac = cpu->ac & mem[fetch_word(cpu) + cpu->xr];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x3e: /* ROL abs,x */
		address = fetch_word(cpu) + cpu->xr;
		mem[address] = rol(cpu, mem[address]);
		break;		
	case 0x40: /* RTI */
		cpu->sr = pull_byte(cpu);
		cpu->sr &= (~break_flag);
		cpu->pc = pull_word(cpu);
		break;
	case 0x41: /* EOR (zp,x) */
		zp = fetch_byte(cpu);
		address = mem[zp+cpu->xr] + (0x100 * mem[zp + cpu->xr + 1]);
		cpu->ac = cpu->ac ^ mem[address];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x45: /* EOR zp */
		cpu->ac = cpu->ac ^ mem[fetch_byte(cpu)];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x46: /* LSR zp */
		zp = fetch_byte(cpu);
		mem[zp] = lsr(cpu, mem[zp]);
		break;
	case 0x48: /* PHA */
		push_byte(cpu, cpu->ac);
		break;
	case 0x49: /* EOR # */
		cpu->ac = cpu->ac ^ fetch_byte(cpu);
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x4a: /* LSR */
		cpu->ac = lsr(cpu, cpu->ac);
		break;
	case 0x4c: /* JMP abs */
		cpu->pc = fetch_word(cpu);
		break;
	case 0x4d: /* EOR abs */
		cpu->ac = cpu->ac ^ mem[fetch_word(cpu)];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x50: /* BVC rel */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & overflow_flag) != overflow_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0x51: /* EOR (zp),y */
		zp = fetch_byte(cpu);
		address = mem[zp] + (0x100 * mem[zp + 1]) + cpu->yr;
		cpu->ac = cpu->ac ^ mem[address];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x55: /* EOR zp,x */
		cpu->ac = cpu->ac ^ mem[fetch_byte(cpu) + cpu->xr];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x56: /* LSR zp,x */
		zp = fetch_byte(cpu);
		mem[zp + cpu->xr] = lsr(cpu, mem[zp + cpu->xr]);
		break;
	case 0x58: /* CLI */
		cpu->sr &= (~interrupt_flag);
		break;
	case 0x59: /* EOR abs,y */
		cpu->ac = cpu->ac ^ mem[fetch_word(cpu) + cpu->yr];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x5d: /* EOR abs,x */
		cpu->ac = cpu->ac ^ mem[fetch_word(cpu) + cpu->xr];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x60: /* RTS */
		// printf("rts\n");
		cpu->pc = pull_word(cpu) + 1;
		break;
	case 0x61: /* ADC (zp,x) */
		zp = fetch_byte(cpu);
		address = mem[zp+cpu->xr] + (0x100 * mem[zp + cpu->xr + 1]);
		cpu->ac = adc(cpu, cpu->ac, mem[address]);
		break;
	case 0x65: /* ADC zp */
		cpu->ac = adc(cpu, cpu->ac, mem[fetch_byte(cpu)]);
		break;
	case 0x66: /* ROR zp */
		zp = fetch_byte(cpu);
		mem[zp] = ror(cpu, mem[zp]);
		break;
	case 0x68: /* PLA */
		cpu->ac = pull_byte(cpu);
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x69: /* ADC # */
		cpu->ac = adc(cpu, cpu->ac, fetch_byte(cpu));
		break;
	case 0x6a: /* ROR */
		cpu->ac = ror(cpu, cpu->ac);
		break;
	case 0x6c: /* JMP (ind) */
		address = fetch_word(cpu);
		/* jmp indirect has a bug */
		i16 bug = ((address & 0xff) == 0xff) ? 0x100 : 0;
		cpu->pc = mem[address] + (0x100 * mem[address + 1 - bug]);
		break;
	case 0x6d: /* ADC abs */
		cpu->ac = adc(cpu, cpu->ac, mem[fetch_word(cpu)]);
		break;
	case 0x70: /* BVS rel */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & overflow_flag) == overflow_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0x71: /* ADC (zp),y */
		zp = fetch_byte(cpu);
		address = mem[zp] + (0x100 * mem[zp + 1]) + cpu->yr;
		cpu->ac = adc(cpu, cpu->ac, mem[address]);
		break;
	case 0x75: /* ADC zp,x */
		cpu->ac = adc(cpu, cpu->ac, mem[fetch_byte(cpu) + cpu->xr]);
		break;
	case 0x76: /* ROR zp,x */
		zp = fetch_byte(cpu);
		mem[zp + cpu->xr] = ror(cpu, mem[zp + cpu->xr]);
		break;
	case 0x78: /* SEI */
		cpu->sr |= interrupt_flag;
		break;
	case 0x79: /* ADC abs,y */
		cpu->ac = adc(cpu, cpu->ac, mem[fetch_word(cpu) + cpu->yr]);
		break;
	case 0x7d: /* ADC abs,x */
		cpu->ac = adc(cpu, cpu->ac, mem[fetch_word(cpu) + cpu->xr]);
		break;
	case 0x81: /* STA (zp, x) */
		zp = fetch_byte(cpu);
		address = mem[zp+cpu->xr] + (0x100 * mem[zp + cpu->xr + 1]);
		mem[address] = cpu->ac;
		break;
	case 0x84: /* STY zp */
		mem[fetch_byte(cpu)] = cpu->yr;
		break;
	case 0x85: /* STA zp */
		mem[fetch_byte(cpu)] = cpu->ac;
		break;
	case 0x86: /* STX zp */
		mem[fetch_byte(cpu)] = cpu->xr;
		break;
	case 0x88: /* DEY */
		cpu->yr = cpu->yr - 1;
		set_nz_flags(cpu, cpu->yr);
		break;
	case 0x8a: /* TXA */
		cpu->ac = cpu->xr;
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x8c: /* STY abs */
		mem[fetch_word(cpu)] = cpu->yr;
		break;
	case 0x8d: /* STA abs */
		mem[fetch_word(cpu)] = cpu->ac;
		break;
	case 0x90: /* BCC rel */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & carry_flag) != carry_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0x91: /* STA (zp),y */
		zp = fetch_byte(cpu);
		address = mem[zp] + (0x100 * mem[zp + 1]) + cpu->yr;
		mem[address] = cpu->ac;
		break;
	case 0x94: /* STY zp,x */
		mem[fetch_byte(cpu) + cpu->xr] = cpu->yr;
		break;
	case 0x95: /* STA zp,x */
		mem[fetch_byte(cpu) + cpu->xr] = cpu->ac;
		break;
	case 0x96: /* STX zp,y */
		mem[fetch_byte(cpu) + cpu->yr] = cpu->xr;
		break;
	case 0x98: /* TYA */
		cpu->ac = cpu->yr;
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0x99: /* STA abs,y */
		mem[fetch_word(cpu) + cpu->yr] = cpu->ac;
		break;
	case 0x9a: /* TXS */
		cpu->sp = cpu->xr;
		break;
	case 0x9d: /* STA abs,x */
		mem[fetch_word(cpu) + cpu->xr] = cpu->ac;
		break;
	case 0xa0: /* LDY # */
		// printf("ldy\n");
		cpu->yr = fetch_byte(cpu);
		set_nz_flags(cpu, cpu->yr);
		break;
	case 0xa1: /* LDA (zp,x) */
		zp = fetch_byte(cpu);
		address = mem[zp+cpu->xr] + (0x100 * mem[zp + cpu->xr + 1]);
		cpu->ac = mem[address];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0xa2: /* LDX # */
		// printf("ldx\n");
		cpu->xr = fetch_byte(cpu);
		set_nz_flags(cpu, cpu->xr);
		break;
	case 0xa4: /* LDY zp */
		cpu->yr = mem[fetch_byte(cpu)];
		set_nz_flags(cpu, cpu->yr);
		break;
	case 0xa5: /* LDA zp */
		cpu->ac = mem[fetch_byte(cpu)];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0xa6: /* LDX zp */
		cpu->xr = mem[fetch_byte(cpu)];
		set_nz_flags(cpu, cpu->xr);
		break;
	case 0xa8: /* TAY */
		cpu->yr = cpu->ac;
		set_nz_flags(cpu, cpu->yr);
		break;
	case 0xa9: /* LDA # */
		cpu->ac = fetch_byte(cpu);
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0xaa: /* TAX */
		cpu->xr = cpu->ac;
		set_nz_flags(cpu, cpu->xr);
		break;
	case 0xac: /* LDY abs */
		cpu->yr = mem[fetch_word(cpu)];
		set_nz_flags(cpu, cpu->yr);
		break;
	case 0xad: /* LDA abs */
		cpu->ac = mem[fetch_word(cpu)];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0xb0: /* BCS rel */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & carry_flag) == carry_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0xb1: /* LDA (zp),y */
		zp = fetch_byte(cpu);
		address = mem[zp] + (0x100 * mem[zp + 1]) + cpu->yr;
		cpu->ac = mem[address];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0xb4: /* LDY zp,x */
		address = fetch_byte(cpu) + cpu->xr;
		cpu->yr = mem[address];
		set_nz_flags(cpu, cpu->yr);
		break;
	case 0xb5: /* LDA zp,x */
		cpu->ac = mem[fetch_byte(cpu) + cpu->xr];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0xb6: /* LDX zp,y */
		cpu->xr = mem[fetch_byte(cpu) + cpu->yr];
		set_nz_flags(cpu, cpu->yr);
		break;
	case 0xb8: /* CLV */
		cpu->sr &= (~overflow_flag);
		break;
	case 0xb9: /* LDA abs,y */
		cpu->ac = mem[fetch_word(cpu) + cpu->yr];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0xba: /* TSX */
		cpu->xr = cpu->sp;
		set_nz_flags(cpu, cpu->xr);
		break;
	case 0xbc: /* LDY abs,x */
		cpu->yr = mem[fetch_word(cpu) + cpu->xr];
		set_nz_flags(cpu, cpu->yr);
		break;
	case 0xbd: /* LDA abs,x */
		cpu->ac = mem[fetch_word(cpu) + cpu->xr];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0xc0: /* CPY # */
		set_cmp_flags(cpu, cpu->yr, fetch_byte(cpu));
		break;
	case 0xc4: /* CPY zp */
		set_cmp_flags(cpu, cpu->yr, mem[fetch_byte(cpu)]);
		break;
	case 0xc1: /* CMP (zp,x) */
		zp = fetch_byte(cpu);
		address = mem[zp+cpu->xr] + (0x100 * mem[zp + cpu->xr + 1]);
		set_cmp_flags(cpu, cpu->ac, mem[address]);
		break;
	case 0xc5: /* CMP zp */
		set_cmp_flags(cpu, cpu->ac, mem[fetch_byte(cpu)]);
		break;
	case 0xc6: /* DEC zp */
		zp = fetch_byte(cpu);
		mem[zp] = mem[zp] - 1;
		set_nz_flags(cpu, mem[zp]);
		break;
	case 0xc8: /* INY */
		cpu->yr = cpu->yr + 1;
		set_nz_flags(cpu, cpu->yr);
		break;
	case 0xc9: /* CMP # */
		set_cmp_flags(cpu, cpu->ac, fetch_byte(cpu));
		break;
	case 0xca: /* DEX */
		cpu->xr = cpu->xr - 1;
		set_nz_flags(cpu, cpu->xr);
		break;
	case 0xcc: /* CPY abs */
		set_cmp_flags(cpu, cpu->yr, mem[fetch_word(cpu)]);
		break;
	case 0xcd: /* CMP abs */
		set_cmp_flags(cpu, cpu->ac, mem[fetch_word(cpu)]);
		break;
	case 0xd0: /* BNE */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & zero_flag) != zero_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0xd1: /* CMP (zp),y */
		zp = fetch_byte(cpu);
		address = mem[zp] + (0x100 * mem[zp + 1]) + cpu->yr;
		set_cmp_flags(cpu, cpu->ac, mem[address]);
		break;
	case 0xd5: /* CMP zp,x */
		set_cmp_flags(cpu, cpu->ac, mem[fetch_byte(cpu) + cpu->xr]);
		break;
	case 0xd6: /* DEC zp,x */
		zp = fetch_byte(cpu) + cpu->xr;
		mem[zp] = mem[zp] - 1;
		set_nz_flags(cpu, mem[zp]);
		break;
	case 0xd8: /* CLD */
		cpu->sr &= (~decimal_flag);
		break;
	case 0xd9: /* CMP abs,y */
		set_cmp_flags(cpu, cpu->ac, mem[fetch_word(cpu) + cpu->yr]);
		break;
	case 0xdd: /* CMP abs,x */
		set_cmp_flags(cpu, cpu->ac, mem[fetch_word(cpu) + cpu->xr]);
		break;
	case 0xe0: /* CPX # */
		set_cmp_flags(cpu, cpu->xr, fetch_byte(cpu));
		break;
	case 0xe1: /* SBC (zp,x) */
		zp = fetch_byte(cpu);
		address = mem[zp+cpu->xr] + (0x100 * mem[zp + cpu->xr + 1]);
		cpu->ac = sbc(cpu, cpu->ac, mem[address]);
		break;
	case 0xe4: /* CPX zp */
		set_cmp_flags(cpu, cpu->xr, mem[fetch_byte(cpu)]);
		break;
	case 0xe5: /* SBC zp */
		cpu->ac = sbc(cpu, cpu->ac, mem[fetch_byte(cpu)]);
		break;
	case 0xe6: /* INC zp */
		zp = fetch_byte(cpu);
		mem[zp] = mem[zp] + 1;
		set_nz_flags(cpu, mem[zp]);
		break;
	case 0xe8: /* INX */
		cpu->xr = cpu->xr + 1;
		set_nz_flags(cpu, cpu->xr);
		break;
	case 0xe9: /* SBC # */
		cpu->ac = sbc(cpu, cpu->ac, fetch_byte(cpu));
		break;
	case 0xea: /* NOP */
		break;
	case 0xec: /* CPX abs */
		set_cmp_flags(cpu, cpu->xr, mem[fetch_word(cpu)]);
		break;
	case 0xed: /* SBC abs */
		cpu->ac = sbc(cpu, cpu->ac, mem[fetch_word(cpu)]);
		break;
	case 0xf0: /* BEQ */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & zero_flag) == zero_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0xf1: /* SBC (zp),y */
		zp = fetch_byte(cpu);
		address = mem[zp] + (0x100 * mem[zp + 1]) + cpu->yr;
		cpu->ac = sbc(cpu, cpu->ac, mem[address]);
		break;
	case 0xf5: /* SBC zp,x */
		cpu->ac = sbc(cpu, cpu->ac, mem[fetch_byte(cpu) + cpu->xr]);
		break;
	case 0xf6: /* INC zp,x */
		zp = fetch_byte(cpu) + cpu->xr;
		mem[zp] = mem[zp] + 1;
		set_nz_flags(cpu, mem[zp]);
		break;
	case 0xf8: /* SED */
		cpu->sr |= decimal_flag;
		break;
	case 0xf9: /* SBC abs,y */
		cpu->ac = sbc(cpu, cpu->ac, mem[fetch_word(cpu) + cpu->yr]);
		break;
	case 0xfd: /* SBC abs,x */
		cpu->ac = sbc(cpu, cpu->ac, mem[fetch_word(cpu) + cpu->xr]);
		break;
	default:
		printf("Not implemented instruction\n");
		cpu->halt = true;
		break;
	}
	
}

void execute_program(CPU_CTX *cpu)
{
	while (!cpu->halt) {
		execute_instruction(cpu);
	}
}
