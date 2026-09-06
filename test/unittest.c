#include "acutest.h"
#include "../cpu.h"

void test_zero_flag(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	set_nz_flags(&cpu, 0x0);
	TEST_CHECK((cpu.sr & zero_flag) == zero_flag);
	set_nz_flags(&cpu, 0x10);
	TEST_CHECK((cpu.sr & zero_flag) == 0x0);
}

void test_negative_flag(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	set_nz_flags(&cpu, 0x80);
	TEST_CHECK((cpu.sr & negative_flag) == negative_flag);
	set_nz_flags(&cpu, 0x7f);
	TEST_CHECK((cpu.sr & negative_flag) == 0x0);
}

void test_bpl(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x10; /* bpl */
	mem[0x101] = 0x20; /* offset */
	cpu.pc = 0x100;
	cpu.sr = negative_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x122);
	// printf("cpu.pc: %02x\n", cpu.pc);
}

void test_jsr(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x20; /* jsr */
	mem[0x101] = 0x22; /* address */
	mem[0x102] = 0xfd;
	cpu.pc = 0x100;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0xfd22);
	TEST_CHECK(mem[0x1ff] == 0x01);
	TEST_CHECK(mem[0x1fe] == 0x02);
	TEST_CHECK(cpu.sp == 0xfd);
}

void test_bmi(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x30; /* bmi */
	mem[0x101] = 0xfe; /* offset */
	cpu.pc = 0x100;
	cpu.sr = negative_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x100);
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
}

void test_bvc(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x50; /* bvc */
	mem[0x101] = 0xfd; /* offset */
	cpu.pc = 0x100;
	cpu.sr = overflow_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0xff);
}

void test_rts(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x60; /* rts */
	cpu.sp = 0xfd; /* set stack pointer */
	mem[0x1fe] = 0x21; /* address -1 in stack */
	mem[0x1ff] = 0xfd;
	cpu.pc = 0x100;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0xfd22);
	TEST_CHECK(cpu.sp == 0xff);
}

void test_bvs(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x70; /* bvs */
	mem[0x101] = 0xfd; /* offset */
	cpu.pc = 0x100;
	cpu.sr = overflow_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0xff);
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
}

TEST_LIST = {
	{"zero flag", test_zero_flag },
	{"negative flag", test_negative_flag },
	{"bpl", test_bpl},
	{"jsr", test_jsr},
	{"bmi", test_bmi},
	{"bvc", test_bvc},
	{"rts", test_rts},
	{"bvs", test_bvs},
	{ NULL, NULL }
};
