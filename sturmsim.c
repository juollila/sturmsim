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
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "cpu.h"
#include "sturmsim.h"
#include "kernal.h"

static SIM_CTX sim_ctx;

static void clean_up(void)
{
	if (sim_ctx.program_file_name) free(sim_ctx.program_file_name);
}

static void print_usage_msg()
{
	const char *usage_msg = "Usage: sturmsim [options] file\n\
 Long options:\n\
  --execute-address=<value>		Specify execution start address. Default value is read from the beginning of PRG file.\n\
  --help				Display available options.\n\
  --lower-case				Enable lower case (shifted) PETSCII.\n\
";
	printf("%s", usage_msg);
	clean_up();
}

static void exit_with_msg(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	clean_up();
	exit(1);
}


static void parse_parameters(int argc, const char *argv[])
{
	const char *execution_address_switch = "--execution-address=";
	sim_ctx.program_file_name = NULL;
	sim_ctx.lower_case = false;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			if (sim_ctx.program_file_name == NULL)
				sim_ctx.program_file_name = strdup(argv[i]);
			else
				exit_with_msg("Too many file names!");
		} else if (strcmp(argv[i], "--lower-case") == 0) {
			sim_ctx.lower_case = true;
		} else if (strcmp(argv[i], "--help") == 0) {
			print_usage_msg();
			clean_up();
			exit(0);
		} else if (strncmp(argv[i], execution_address_switch, strlen(execution_address_switch)) == 0) {
			sim_ctx.execution_address = true;
			char substring[10];
			strncpy(substring, argv[i] + strlen(execution_address_switch), 6);
			sim_ctx.execution_address_value = (unsigned short) strtoul(substring, NULL, 16);
		} else exit_with_msg("Unknown option");
	}	
	if (sim_ctx.program_file_name == NULL)
		exit_with_msg("File name is missing!");
}

u16 load_program(const char *file_name)
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
	return start_address;

}

int main(int argc, const char *argv[])
{
	u16 load_address;
	parse_parameters(argc, argv);
	CPU_CTX cpu;
	init_cpu(&cpu);
	init_memory();
	init_kernal();
	load_address = load_program(sim_ctx.program_file_name);
	cpu.pc = sim_ctx.execution_address ? sim_ctx.execution_address_value : load_address;
	clean_up();
	execute_program(&sim_ctx, &cpu);
	return 0;
}
