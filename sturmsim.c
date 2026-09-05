#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "sturmsim.h"

#define MEM_SIZE 65536

CTX ctx;
u8 mem[MEM_SIZE];

static void clean_up(void)
{
	if (ctx.program_file_name) free(ctx.program_file_name);
}

static void exit_with_usage_msg(char *msg)
{
	fprintf(stderr, "%s\n", msg);
	fprintf(stderr, "usage: sturmsim <file>\n");
	clean_up();
	exit(1);
}

static void exit_with_msg(char *msg)
{
	fprintf(stderr, "%s\n", msg);
	clean_up();
	exit(1);
}


static void parse_parameters(int argc, char *argv[])
{
	ctx.program_file_name = NULL;

	if(argc < 2) {
		exit_with_usage_msg("File name is missing!");
	}
	ctx.program_file_name = strdup(argv[1]);
}

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

u16 load_program(char *file_name)
{
	int low, high, ch;
	u32 address;
	u16 start_address;

	FILE *program = fopen(file_name, "rb");
	if (program == NULL) {
		exit_with_msg("Cannot open program file!");
	}

	low = fgetc(program);
	high = fgetc(program);

	if (low != EOF && high != EOF) {
		address = (high * 256) + low;
		start_address = address;
		while (( ch = fgetc(program)) != EOF) {
			if (address >= MEM_SIZE) {
				fclose(program);
				exit_with_msg("Program too large!");
			}
			mem[address++] = (u8) ch;
		}
	} else {
		fclose(program);
		exit_with_msg("Cannot read start address.");
	}
	fclose(program);
	// printf("start_address: %04x\n", start_address);
	// printf("end_address: %04x\n", address-1);
	return start_address;

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

static void set_nz_flags(CPU_CTX *cpu, u8 i)
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
	case 0x20: /* JSR abs */
		push_word(cpu, cpu->pc + 1);
		cpu->pc = fetch_word(cpu);
		// printf("jsr %04x\n", cpu->pc);
		break;
	case 0x60: /* RTS */
		// printf("rts\n");
		cpu->pc = pull_word(cpu) + 1;
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

int main(int argc, char *argv[])
{
	u16 load_address;
	parse_parameters(argc, argv);
	CPU_CTX cpu;
	init_cpu(&cpu);
	init_memory();
	load_address = load_program(ctx.program_file_name);
	cpu.pc = load_address;
	clean_up();
	execute_program(&cpu);
	return 0;
}
