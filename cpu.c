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
	cpu->sr = one_flag;
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
	case 0x1: /* ORA (zp, X) */
		zp = fetch_byte(cpu);
		address = mem[zp+cpu->xr] + (0x100 * mem[zp + cpu->xr + 1]);
		cpu->ac = cpu->ac | mem[address];
		set_nz_flags(cpu, cpu->ac);
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
	case 0x20: /* JSR abs */
		push_word(cpu, cpu->pc + 1);
		cpu->pc = fetch_word(cpu);
		// printf("jsr %04x\n", cpu->pc);
		break;
	case 0x21: /* AND (zp, X) */
		zp = fetch_byte(cpu);
		address = mem[zp+cpu->xr] + (0x100 * mem[zp + cpu->xr + 1]);
		cpu->ac = cpu->ac & mem[address];
		set_nz_flags(cpu, cpu->ac);
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
	case 0x40: /* RTI */
		cpu->sr = pull_byte(cpu);
		cpu->sr &= (~break_flag);
		cpu->pc = pull_word(cpu);
		break;
	case 0x50: /* BVC rel */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & overflow_flag) != overflow_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0x60: /* RTS */
		// printf("rts\n");
		cpu->pc = pull_word(cpu) + 1;
		break;
	case 0x70: /* BVS rel */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & overflow_flag) == overflow_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0x90: /* BCC rel */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & carry_flag) != carry_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0xa0: /* LDY # */
		// printf("ldy\n");
		cpu->yr = fetch_byte(cpu);
		set_nz_flags(cpu, cpu->yr);
		break;
	case 0xa2: /* LDX # */
		// printf("ldx\n");
		cpu->xr = fetch_byte(cpu);
		set_nz_flags(cpu, cpu->xr);
		break;
	case 0xa9: /* LDA # */
		// printf("lda\n");
		cpu->ac = fetch_byte(cpu);
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0xb0: /* BCS rel */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & carry_flag) == carry_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0xbd: /* LDA abs,X */
		// printf("lda abs,x\n");
		cpu->ac = mem[fetch_word(cpu) + cpu->xr];
		set_nz_flags(cpu, cpu->ac);
		break;
	case 0xc0: /* CPY # */
		set_cmp_flags(cpu, cpu->yr, fetch_byte(cpu));
		break;
	case 0xd0: /* BNE */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & zero_flag) != zero_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0xe0: /* CPX # */
		set_cmp_flags(cpu, cpu->xr, fetch_byte(cpu));
		break;
	case 0xe8: /* INX */
		cpu->xr = cpu->xr + 1;
		set_nz_flags(cpu, cpu->xr);
		break;
	case 0xf0: /* BEQ */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & zero_flag) == zero_flag) cpu->pc = cpu->pc + offset;
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
