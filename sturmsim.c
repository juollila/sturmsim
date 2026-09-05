#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "cpu.h"
#include "sturmsim.h"

CTX ctx;

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
