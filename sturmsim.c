#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "sturmsim.h"

#define MEM_SIZE 65536

CTX ctx;
u8 mem[MEM_SIZE];

void clean_up(void)
{
	if (ctx.program_file_name) free(ctx.program_file_name);
}

void exit_with_usage_msg(char *msg)
{
	fprintf(stderr, "%s\n", msg);
	fprintf(stderr, "usage: sturmsim <file>\n");
	clean_up();
	exit(1);
}

void exit_with_msg(char *msg)
{
	fprintf(stderr, "%s\n", msg);
	clean_up();
	exit(1);
}


void parse_parameters(int argc, char *argv[])
{
	ctx.program_file_name = NULL;

	if(argc < 2) {
		exit_with_usage_msg("File name is missing!");
	}
	ctx.program_file_name = strdup(argv[1]);
}

void init_cpu(CPU_CTX *cpu)
{
	cpu->a = 0;
	cpu->x = 0;
	cpu->y = 0;
	cpu->sp = 0xff;
	cpu->st = one_flag;
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

u8 fetch_byte(CPU_CTX *cpu)
{
	return mem[cpu->pc++];
}

u8 peek_byte(CPU_CTX *cpu)
{
	return mem[cpu->pc];
}

u16 fetch_word(CPU_CTX *cpu)
{
	return (mem[cpu->pc++] + 256 * mem[cpu->pc++]);
}

u16 peek_word(CPU_CTX *cpu)
{
	return (mem[cpu->pc] + 256 * mem[cpu->pc+1]);
}

void push_byte(CPU_CTX *cpu, u8 i)
{
	mem[0x100 + cpu->sp] = i;
	cpu->sp--;
}

void push_word(CPU_CTX *cpu, u16 i)
{
	mem[0x100 + cpu->sp] = ((i >> 8) & 0xff);
	cpu->sp--;
	mem[0x100 + cpu->sp] = (i & 0xff);
	cpu->sp--;
}

u8 pull_byte(CPU_CTX *cpu)
{
	cpu->sp++;
	return mem[0x100 + cpu->sp];
}

u16 pull_word(CPU_CTX *cpu)
{
	u16 value;
	cpu->sp++;
	value = mem[0x100 + cpu->sp];
	cpu->sp++;
	value = value + (0x100 * mem[0x100 + cpu->sp]);
	// printf("pull_word: %04x\n", value);
	return value;	
}

void chrout(u8 i)
{
	if (i == 0xd) i = '\n';
	if (i >= 0x60) i = '.'; 
	putchar(i);
}

void set_nz_flags(CPU_CTX *cpu, u8 i)
{
	if (i == 0) cpu->st |= zero_flag; else cpu->st &= (~zero_flag);
	if (i >= 0x80) cpu->st |= negative_flag; else cpu->st &= (~negative_flag);
}

void execute_instruction(CPU_CTX *cpu)
{
	u8 instruction;
	i8 offset;
	u16 address;

	if (cpu->pc == 0xffd2) {
		chrout(cpu->a);
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
		cpu->y = fetch_byte(cpu);
		set_nz_flags(cpu, cpu->y);
		break;
	case 0xa2: /* LDX # */
		// printf("ldx\n");
		cpu->x = fetch_byte(cpu);
		set_nz_flags(cpu, cpu->x);
		break;
	case 0xa9: /* LDA # */
		// printf("lda\n");
		cpu->a = fetch_byte(cpu);
		set_nz_flags(cpu, cpu->a);
		break;
	case 0xbd: /* LDA abs,X */
		// printf("lda abs,x\n");
		cpu->a = mem[fetch_word(cpu) + cpu->x];
		set_nz_flags(cpu, cpu->a);
		// printf("PC: %04x ST: %02x SP: %04x ACC: %02x X: %02x Y: %02x\n", cpu->pc, cpu->st, (0x100 + cpu->sp), cpu->a, cpu->x, cpu->y);
		break;
	case 0xd0: /* BNE */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->st & zero_flag) != zero_flag) cpu->pc = cpu->pc + offset;
		// printf("bne cpu->st: %02x cpu->pc: %04x\n", cpu->st, cpu->pc);
		// printf("PC: %04x ST: %02x SP: %04x ACC: %02x X: %02x Y: %02x\n", cpu->pc, cpu->st, (0x100 + cpu->sp), cpu->a, cpu->x, cpu->y);
		break;
	case 0xe8: /* INX */
		cpu->x = cpu->x + 1;
		set_nz_flags(cpu, cpu->x);
		break;
	case 0xf0: /* BEQ */
		offset = (i8) fetch_byte(cpu);
		if ((cpu->st & zero_flag) == zero_flag) cpu->pc = cpu->pc + offset;
		// printf("beq cpu->st: %02x cpu->pc: %04x\n", cpu->st, cpu->pc);
		// printf("PC: %04x ST: %02x SP: %04x ACC: %02x X: %02x Y: %02x\n", cpu->pc, cpu->st, (0x100 + cpu->sp), cpu->a, cpu->x, cpu->y);
		// cpu->halt = true;
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
	// printf("PC: %04x SP: %04x ACC: %02x X: %02x Y: %02x\n", cpu.pc, (0x100 + cpu.sp), cpu.a, cpu.x, cpu.y);
	// printf("stack: %02x%02x%02x%02x\n", mem[0x1fc], mem[0x1fd], mem[0x1fe], mem[0x1ff]);
	// printf("$0100: %02x%02x%02x%02x %02x%02x%02x%02x\n", mem[0x100], mem[0x101], mem[0x102], mem[0x103], mem[0x104], mem[0x105], mem[0x106], mem[0x107]);
	// printf("$0108: %02x%02x%02x%02x %02x%02x%02x%02x\n", mem[0x108], mem[0x109], mem[0x10a], mem[0x10b], mem[0x10c], mem[0x10d], mem[0x10e], mem[0x10f]);
	return 0;
}
