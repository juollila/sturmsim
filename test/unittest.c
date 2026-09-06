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

void test_cmp_flag(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	/* register < operand */
	set_cmp_flags(&cpu, 0x10, 0x20);
	TEST_CHECK((cpu.sr & zero_flag) == 0x0);
	TEST_CHECK((cpu.sr & carry_flag) == 0x0);
	TEST_CHECK((cpu.sr & negative_flag) == negative_flag);
	set_cmp_flags(&cpu, 0x10, 0xff);
	TEST_CHECK((cpu.sr & zero_flag) == 0x0);
	TEST_CHECK((cpu.sr & carry_flag) == 0x0);
	TEST_CHECK((cpu.sr & negative_flag) == 0x0);
	/* register = operand */
	set_cmp_flags(&cpu, 0x10, 0x10);
	TEST_CHECK((cpu.sr & zero_flag) == zero_flag);
	TEST_CHECK((cpu.sr & carry_flag) == carry_flag);
	TEST_CHECK((cpu.sr & negative_flag) == 0x0);
	/* register > operand */
	set_cmp_flags(&cpu, 0xd0, 0x10);
	TEST_CHECK((cpu.sr & zero_flag) == 0x0);
	TEST_CHECK((cpu.sr & carry_flag) == carry_flag);
	TEST_CHECK((cpu.sr & negative_flag) == negative_flag);
	set_cmp_flags(&cpu, 0x30, 0x10);
	TEST_CHECK((cpu.sr & zero_flag) == 0x0);
	TEST_CHECK((cpu.sr & carry_flag) == carry_flag);
	TEST_CHECK((cpu.sr & negative_flag) == 0x0);
}

void test_brk(void)
{
	CPU_CTX cpu;
	/* default CVINB */
	init_cpu(&cpu);
	init_memory();
	cpu.ac = 0x1;
	cpu.xr = 0x2;
	cpu.yr = 0x3;
	mem[0x110] = 0x0; /* brk */
	cpu.pc = 0x110;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.halt == true);
	TEST_CHECK(cpu.pc == 0x111);
	TEST_CHECK(cpu.sr == break_flag);
	TEST_CHECK(cpu.sp == 0xf9);
	TEST_CHECK(mem[0x1ff] == 0x01);
	TEST_CHECK(mem[0x1fe] == 0x11);
	TEST_CHECK(mem[0x1fd] == break_flag);
	TEST_CHECK(mem[0x1fc] == 0x1);
	TEST_CHECK(mem[0x1fb] == 0x2);
	TEST_CHECK(mem[0x1fa] == 0x3);
	/* modified CVINB */
	init_cpu(&cpu);
	init_memory();
	mem[0x316] = 0x00;
	mem[0x317] = 0x80;
	cpu.ac = 0x1;
	cpu.xr = 0x2;
	cpu.yr = 0x3;
	mem[0x110] = 0x0; /* brk */
	cpu.pc = 0x110;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.halt == false);
	TEST_CHECK(cpu.pc == 0x8000);
	TEST_CHECK(cpu.sr == break_flag);
	TEST_CHECK(cpu.sp == 0xf9);
	TEST_CHECK(mem[0x1ff] == 0x01);
	TEST_CHECK(mem[0x1fe] == 0x11);
	TEST_CHECK(mem[0x1fd] == break_flag);
	TEST_CHECK(mem[0x1fc] == 0x1);
	TEST_CHECK(mem[0x1fb] == 0x2);
	TEST_CHECK(mem[0x1fa] == 0x3);
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

void test_rti(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x40; /* rti */
	cpu.pc = 0x100;
	cpu.sr = 0;
	cpu.sp = 0xfc;
	mem[0x1fd] = (carry_flag | break_flag);
	mem[0x1fe] = 0x22;
	mem[0x1ff] = 0xfd;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0xfd22);
	TEST_CHECK(cpu.sr == carry_flag);
	TEST_CHECK(cpu.sp == 0xff);
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

void test_bcc(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x90; /* bcc */
	mem[0x101] = 0x4e; /* offset */
	cpu.pc = 0x100;
	cpu.sr = carry_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x150);
}

void test_ldy_imm(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0xa0; /* ldy */
	mem[0x101] = 0x4e; /* #$4e */
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.yr == 0x4e);
	TEST_CHECK((cpu.sr) == 0);
	mem[0x100] = 0xa0; /* ldy */
	mem[0x101] = 0x00; /* #$0 */
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.yr == 0x0);
	TEST_CHECK((cpu.sr) == zero_flag);
	mem[0x100] = 0xa0; /* ldy */
	mem[0x101] = 0xff; /* #$ff */
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.yr == 0xff);
	TEST_CHECK((cpu.sr) == negative_flag);
}

void test_bcs(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0xb0; /* bcs */
	mem[0x101] = 0x4e; /* offset */
	cpu.pc = 0x100;
	cpu.sr = carry_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x150);
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
}

void test_cpy_imm(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x10;
	mem[0x100] = 0xc0; /* cpy */
	mem[0x101] = 0x20; /* #$20 */
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.yr == 0x10);
	TEST_CHECK((cpu.sr) == negative_flag);
	mem[0x100] = 0xc0; /* cpy */
	mem[0x101] = 0x10; /* #$10 */
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.yr == 0x10);
	TEST_CHECK((cpu.sr) == (zero_flag | carry_flag));
	mem[0x100] = 0xc0; /* cpy */
	mem[0x101] = 0x01; /* #$01 */
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.yr == 0x10);
	TEST_CHECK((cpu.sr) == carry_flag);
}

void test_bne(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0xd0; /* bne */
	mem[0x101] = 0x80; /* offset */
	cpu.pc = 0x100;
	cpu.sr = zero_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x82);
}

void test_cpx_imm(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x100] = 0xe0; /* cpx */
	mem[0x101] = 0x91; /* #$91 */
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.xr == 0x10);
	TEST_CHECK((cpu.sr) == 0x0);
	mem[0x100] = 0xe0; /* cpx */
	mem[0x101] = 0x10; /* #$10 */
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.xr == 0x10);
	TEST_CHECK((cpu.sr) == (zero_flag | carry_flag));
	cpu.xr = 0x90;
	mem[0x100] = 0xe0; /* cpx */
	mem[0x101] = 0x01; /* #$01 */
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.xr == 0x90);
	TEST_CHECK((cpu.sr) == (carry_flag | negative_flag));
}

void test_beq(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0xf0; /* beq */
	mem[0x101] = 0x80; /* offset */
	cpu.pc = 0x100;
	cpu.sr = zero_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x82);
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
}

TEST_LIST = {
	{"zero flag", test_zero_flag },
	{"negative flag", test_negative_flag },
	{"compare flags", test_cmp_flag },
	{"brk", test_brk},
	{"bpl", test_bpl},
	{"jsr", test_jsr},
	{"bmi", test_bmi},
	{"rti", test_rti},
	{"bvc", test_bvc},
	{"rts", test_rts},
	{"bvs", test_bvs},
	{"bcc", test_bcc},
	{"ldy #", test_ldy_imm},
	{"bcs", test_bcs},
	{"cpy #", test_cpy_imm},
	{"bne", test_bne},
	{"cpx #", test_cpx_imm},
	{"beq", test_beq},
	{ NULL, NULL }
};
