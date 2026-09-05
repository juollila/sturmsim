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

TEST_LIST = {
	{"zero flag", test_zero_flag },
	{"negative flag", test_negative_flag },
	{ NULL, NULL }
};
