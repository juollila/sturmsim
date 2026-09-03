#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "sturmsim.h"

#define MEM_SIZE 65536

CPU cpu;
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

void init_cpu(void)
{
	cpu.a = 0;
	cpu.x = 0;
	cpu.y = 0;
	cpu.sp = 0x1ff;
	cpu.st = one_flag;
	cpu.pc = 0xfffc;
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

void load_program(char *file_name)
{
	char low, high, ch;
	u32 address;

	FILE *program = fopen(file_name, "rb");
	if (program == NULL) {
		exit_with_msg("Cannot open program file!");
	}

	low = fgetc(program);
	high = fgetc(program);

	if (low != EOF && high != EOF) {
		address = (high * 256) + low;
		cpu.pc = address;
		while (( ch = fgetc(program)) != EOF) {
			if (address >= MEM_SIZE) {
				fclose(program);
				exit_with_msg("Program too large!");
			}
			mem[address++] = ch;
		}
	} else {
		fclose(program);
		exit_with_msg("Cannot read start address.");
	}
	fclose(program);

}

u8 fetch_byte(void)
{
	return mem[cpu.pc++];
}

u8 peek_byte(void)
{
	return mem[cpu.pc];
}

u16 fetch_word(void)
{
	return (mem[cpu.pc++] + 256 * mem[cpu.pc++]);
}

u16 peek_word(void)
{
	return (mem[cpu.pc] + 256 * mem[cpu.pc+1]);
}

void execute(void)
{
	u8 instruction, byte;
	u16 word;

	instruction = fetch_byte();
	switch (instruction) {
	case 0xa0: /* LDY # */
		cpu.y = fetch_byte();
		break;
	case 0xa2: /* LDX # */
		cpu.x = fetch_byte();
		break;
	case 0xa9: /* LDA # */
		cpu.a = fetch_byte();
		break;
	default:
		printf("Not implemented instruction\n");
		break;
		// assert(false && "Not implemented instruction");
	}
	
}

int main(int argc, char *argv[])
{
	parse_parameters(argc, argv);
	init_cpu();
	init_memory();
	load_program(ctx.program_file_name);
	clean_up();
	// unsigned int address = 0x100;
	// printf("%04x %02x%02x%02x%02x\n", address, mem[address], mem[address+1], mem[address+2], mem[address+3]);
	execute();
	execute();
	execute();
	execute();
	printf("PC: %04x ACC: %02x X: %02x Y: %02x\n", cpu.pc, cpu.a, cpu.x, cpu.y);
	return 0;
}
