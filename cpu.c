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

static void print_registers(CPU_CTX *cpu)
{
	printf("   PC   SR AC XR YR SP\n");
	printf("; %04x %02x %02x %02x %02x %02x\n", cpu->pc, cpu->sr, cpu->ac, cpu->xr, cpu->yr, cpu->sp);
}

void execute_instruction(CPU_CTX *cpu)
{
	u8 instruction;
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
	case 0x0: /* TODO: Implement proper break support */
		// printf("\n");
		cpu->halt = true;
		break;
	case 0x10: /* BPL rel */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & negative_flag) != negative_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0x20: /* JSR abs */
		push_word(cpu, cpu->pc + 1);
		cpu->pc = fetch_word(cpu);
		// printf("jsr %04x\n", cpu->pc);
		break;
	case 0x30: /* BMI rel */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & negative_flag) == negative_flag) cpu->pc = cpu->pc + offset;
		break;
	case 0x40: /* TODO: RTI */
		cpu->halt = true;
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
	case 0xd0: /* BNE */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->sr & zero_flag) != zero_flag) cpu->pc = cpu->pc + offset;
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
