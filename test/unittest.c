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

void test_cmp_flags(void)
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

void test_adc_flags(void)
{
	CPU_CTX cpu;
	u8 result;
	init_cpu(&cpu);
	cpu.sr = 0;
	result = adc(&cpu, 0x40, 0x40);
	TEST_CHECK(result == 0x80);
	TEST_CHECK((cpu.sr & carry_flag) == 0);
	TEST_CHECK((cpu.sr & overflow_flag) == overflow_flag);
	TEST_CHECK((cpu.sr & negative_flag) == negative_flag);
	TEST_CHECK((cpu.sr & zero_flag) == 0);
	cpu.sr = 0;
	result = adc(&cpu, 0xff, 0x01);
	TEST_CHECK(result == 0x0);
	TEST_CHECK((cpu.sr & carry_flag) == carry_flag);
	TEST_CHECK((cpu.sr & overflow_flag) == 0);
	TEST_CHECK((cpu.sr & negative_flag) == 0);
	TEST_CHECK((cpu.sr & zero_flag) == zero_flag);
	cpu.sr = carry_flag;
	result = adc(&cpu, 0x7f, 0x0);
	TEST_CHECK(result == 0x80);
	TEST_CHECK((cpu.sr & carry_flag) == 0);
	TEST_CHECK((cpu.sr & overflow_flag) == overflow_flag);
	TEST_CHECK((cpu.sr & negative_flag) == negative_flag);
	TEST_CHECK((cpu.sr & zero_flag) == 0);
	cpu.sr = 0;
	result = adc(&cpu, 0x80, 0x80);
	TEST_CHECK(result == 0x0);
	TEST_CHECK((cpu.sr & carry_flag) == carry_flag);
	TEST_CHECK((cpu.sr & overflow_flag) == overflow_flag);
	TEST_CHECK((cpu.sr & negative_flag) == 0);
	TEST_CHECK((cpu.sr & zero_flag) == zero_flag);
	cpu.sr = 0;
	result = adc(&cpu, 0x50, 0x10);
	TEST_CHECK(result == 0x60);
	TEST_CHECK((cpu.sr & carry_flag) == 0);
	TEST_CHECK((cpu.sr & overflow_flag) == 0);
	TEST_CHECK((cpu.sr & negative_flag) == 0);
	TEST_CHECK((cpu.sr & zero_flag) == 0);
	cpu.sr = carry_flag;
	result = adc(&cpu, 0xff, 0xff);
	TEST_CHECK(result == 0xff);
	TEST_CHECK((cpu.sr & carry_flag) == carry_flag);
	TEST_CHECK((cpu.sr & overflow_flag) == 0);
	TEST_CHECK((cpu.sr & negative_flag) == negative_flag);
	TEST_CHECK((cpu.sr & zero_flag) == 0);
}

void test_adc_bcd_flags(void)
{
	CPU_CTX cpu;
	u8 result;
	/* 0x99 + 0x00 */
	init_cpu(&cpu);
	cpu.sr = decimal_flag;
	result = adc(&cpu, 0x99, 0x00);
	TEST_CHECK(result == 0x99);
	TEST_CHECK(cpu.sr == (decimal_flag | negative_flag));
	/* 0x99 + 0x00 + carry */
	cpu.sr = decimal_flag | carry_flag;
	result = adc(&cpu, 0x99, 0x00);
	TEST_CHECK(result == 0x00);
	TEST_CHECK(cpu.sr == (decimal_flag | negative_flag | carry_flag));
	/* 0xaa + 0x11 */
	cpu.sr = decimal_flag;
	result = adc(&cpu, 0xaa, 0x11);
	TEST_CHECK(result == 0x21);
	TEST_CHECK(cpu.sr == (decimal_flag | negative_flag | carry_flag));
	/* 0x15 + 0x27 */
	cpu.sr = decimal_flag;
	result = adc(&cpu, 0x15, 0x27);
	TEST_CHECK(result == 0x42);
	TEST_CHECK(cpu.sr == decimal_flag);
	/* 0x15 + 0x27 + carry */
	cpu.sr = decimal_flag | carry_flag;
	result = adc(&cpu, 0x15, 0x27);
	TEST_CHECK(result == 0x43);
	TEST_CHECK(cpu.sr == decimal_flag);
	/* 0x50 + 0x50 */
	cpu.sr = decimal_flag;
	result = adc(&cpu, 0x50, 0x50);
	TEST_CHECK(result == 0x00);
	TEST_CHECK(cpu.sr == (decimal_flag | negative_flag | overflow_flag | carry_flag));
	/* 0x79 + 0x20 + carry */
	cpu.sr = decimal_flag | carry_flag;
	result = adc(&cpu, 0x79, 0x20);
	TEST_CHECK(result == 0x00);
	TEST_CHECK(cpu.sr == (decimal_flag | negative_flag | overflow_flag | carry_flag));
}

void test_sbc_flags(void)
{
	CPU_CTX cpu;
	u8 result;
	init_cpu(&cpu);
	cpu.sr = carry_flag;
	result = sbc(&cpu, 0x0, 0x0);
	TEST_CHECK(result == 0x0);
	TEST_CHECK((cpu.sr & negative_flag) == 0);
	TEST_CHECK((cpu.sr & zero_flag) == zero_flag);
	TEST_CHECK((cpu.sr & carry_flag) == carry_flag);
	TEST_CHECK((cpu.sr & overflow_flag) == 0);
	cpu.sr = carry_flag;
	result = sbc(&cpu, 0x0, 0x1);
	TEST_CHECK(result == 0xff);
	TEST_CHECK((cpu.sr & negative_flag) == negative_flag);
	TEST_CHECK((cpu.sr & zero_flag) == 0);
	TEST_CHECK((cpu.sr & carry_flag) == 0);
	TEST_CHECK((cpu.sr & overflow_flag) == 0);
	cpu.sr = carry_flag;
	result = sbc(&cpu, 0xff, 0x1);
	TEST_CHECK(result == 0xfe);
	TEST_CHECK((cpu.sr & negative_flag) == negative_flag);
	TEST_CHECK((cpu.sr & zero_flag) == 0);
	TEST_CHECK((cpu.sr & carry_flag) == carry_flag);
	TEST_CHECK((cpu.sr & overflow_flag) == 0);
	cpu.sr = 0;
	result = sbc(&cpu, 0x1, 0x0);
	TEST_CHECK(result == 0x0);
	TEST_CHECK((cpu.sr & negative_flag) == 0);
	TEST_CHECK((cpu.sr & zero_flag) == zero_flag);
	TEST_CHECK((cpu.sr & carry_flag) == carry_flag);
	TEST_CHECK((cpu.sr & overflow_flag) == 0);
	cpu.sr = 0;
	result = sbc(&cpu, 0x0, 0x0);
	TEST_CHECK(result == 0xff);
	TEST_CHECK((cpu.sr & negative_flag) == negative_flag);
	TEST_CHECK((cpu.sr & zero_flag) == 0);
	TEST_CHECK((cpu.sr & carry_flag) == 0);
	TEST_CHECK((cpu.sr & overflow_flag) == 0);
	cpu.sr = carry_flag;
	result = sbc(&cpu, 0x80, 0x1);
	TEST_CHECK(result == 0x7f);
	TEST_CHECK((cpu.sr & negative_flag) == 0);
	TEST_CHECK((cpu.sr & zero_flag) == 0);
	TEST_CHECK((cpu.sr & carry_flag) == carry_flag);
	TEST_CHECK((cpu.sr & overflow_flag) == overflow_flag);
	cpu.sr = carry_flag;
	result = sbc(&cpu, 0x7f, 0xff);
	TEST_CHECK(result == 0x80);
	TEST_CHECK((cpu.sr & negative_flag) == negative_flag);
	TEST_CHECK((cpu.sr & zero_flag) == 0);
	TEST_CHECK((cpu.sr & carry_flag) == 0);
	TEST_CHECK((cpu.sr & overflow_flag) == overflow_flag);
	cpu.sr = carry_flag;
	result = sbc(&cpu, 0x80, 0x7f);
	TEST_CHECK(result == 0x1);
	TEST_CHECK((cpu.sr & negative_flag) == 0);
	TEST_CHECK((cpu.sr & zero_flag) == 0);
	TEST_CHECK((cpu.sr & carry_flag) == carry_flag);
	TEST_CHECK((cpu.sr & overflow_flag) == overflow_flag);
}

void test_sbc_bcd_flags(void)
{
	CPU_CTX cpu;
	u8 result;
	/* 0x50 - 0x1 (no borrow) */
	init_cpu(&cpu);
	cpu.sr = decimal_flag | carry_flag;
	result = sbc(&cpu, 0x50, 0x1);
	TEST_CHECK(result == 0x49);
	TEST_CHECK(cpu.sr == (decimal_flag | carry_flag));
	/* 0x0 - 0x1 (no borrow) */
	cpu.sr = decimal_flag | carry_flag;
	result = sbc(&cpu, 0x0, 0x1);
	TEST_CHECK(result == 0x99);
	TEST_CHECK(cpu.sr == (decimal_flag | negative_flag));
	/* 0x99 - 0x1 (no borrow) */
	cpu.sr = decimal_flag | carry_flag;
	result = sbc(&cpu, 0x99, 0x1);
	TEST_CHECK(result == 0x98);
	TEST_CHECK(cpu.sr == (decimal_flag | negative_flag | carry_flag));
	/* 0x10 - 0x1 (borrow) */
	cpu.sr = decimal_flag;
	result = sbc(&cpu, 0x10, 0x1);
	TEST_CHECK(result == 0x8);
	TEST_CHECK(cpu.sr == (decimal_flag | carry_flag));
	/* 0x80 - 0x1 (no borrow) */
	cpu.sr = decimal_flag | carry_flag;
	result = sbc(&cpu, 0x80, 0x1);
	TEST_CHECK(result == 0x79);
	TEST_CHECK(cpu.sr == (decimal_flag | overflow_flag | carry_flag));
	/* 0x50 - 0x50 (no borrow) */
	cpu.sr = decimal_flag | carry_flag;
	result = sbc(&cpu, 0x50, 0x50);
	TEST_CHECK(result == 0x0);
	TEST_CHECK(cpu.sr == (decimal_flag | zero_flag | carry_flag));
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

void test_ora_zp_x_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x100] = 0x01; /* ora ($10, x) */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8000] = 0xf;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.xr == 0x10);
	TEST_CHECK((cpu.sr) == 0x0);
	TEST_CHECK((cpu.ac) == 0x5f);
}

void test_ora_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x20;
	mem[0x10] = 0x2;
	mem[0x100] = 0x5; /* ora $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == 0);
	TEST_CHECK(cpu.ac == 0x22);
}

void test_asl_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x10] = 0x80;
	mem[0x100] = 0x6; /* asl $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == (carry_flag | zero_flag));
	TEST_CHECK(mem[0x10] == 0x0);
}

void test_php(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x8; /* php */
	cpu.pc = 0x100;
	cpu.sr = negative_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sp == 0xfe);
	TEST_CHECK(mem[0x1ff] == (break_flag | one_flag | negative_flag));
}

void test_ora_imm(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x09; /* ora #80 */
	mem[0x101] = 0x80;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x8;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK((cpu.sr) == negative_flag);
	TEST_CHECK((cpu.ac) == 0x88);
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
}


void test_ora_zp_y_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x10;
	mem[0x100] = 0x11; /* ora ($20), y */
	mem[0x101] = 0x20;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8010] = 0xf0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.yr == 0x10);
	TEST_CHECK((cpu.sr) == negative_flag);
	TEST_CHECK((cpu.ac) == 0xf5);
}

void test_ora_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x08;
	cpu.xr = 0x10;
	mem[0x20] = 0x80;
	mem[0x100] = 0x15; /* ora $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == negative_flag);
	TEST_CHECK(cpu.ac == 0x88);
}

void test_asl_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x20] = 0x40;
	mem[0x100] = 0x16; /* asl $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == negative_flag);
	TEST_CHECK(mem[0x20] == 0x80);
}

void test_clc(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x18; /* clc */
	cpu.pc = 0x100;
	cpu.sr = (negative_flag | carry_flag);
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sr == negative_flag);
}

void test_ora_abs_y(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x19; /* ora $1001,y */
	mem[0x101] = 0x01;
	mem[0x102] = 0x10;
	mem[0x1011] = 0xf;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.yr = 0x10;
	cpu.ac = 0x70;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x103);
	TEST_CHECK((cpu.sr) == 0x0);
	TEST_CHECK((cpu.ac) == 0x7f);
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

void test_and_zp_x_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x100] = 0x21; /* and ($10, x) */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8000] = 0xf;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.xr == 0x10);
	TEST_CHECK((cpu.sr) == 0x0);
	TEST_CHECK((cpu.ac) == 0x05);
}

void test_bit_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x24; /* bit zp */
	mem[0x101] = 0x20;
	mem[0x20] = 0x7f;
	cpu.pc = 0x100;
	cpu.ac = 0x80;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == (zero_flag | overflow_flag));
	mem[0x100] = 0x24; /* bit zp */
	mem[0x101] = 0x20;
	mem[0x20] = 0x80;
	cpu.pc = 0x100;
	cpu.ac = 0x80;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == negative_flag);
}

void test_and_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x08;
	mem[0x10] = 0x80;
	mem[0x100] = 0x25; /* and $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == zero_flag);
	TEST_CHECK(cpu.ac == 0x0);
}

void test_rol_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x10] = 0x81;
	mem[0x100] = 0x26; /* rol $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == carry_flag);
	TEST_CHECK(mem[0x10] == 0x2);
}

void test_plp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x28; /* plp */
	cpu.pc = 0x100;
	cpu.sp = 0xfe;
	mem[0x1ff] = (negative_flag | carry_flag);
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sp == 0xff);
	TEST_CHECK(cpu.sr == (negative_flag | carry_flag));
}

void test_and_imm(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x29; /* and #80 */
	mem[0x101] = 0x80;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x8;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK((cpu.sr) == zero_flag);
	TEST_CHECK((cpu.ac) == 0x0);
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

void test_and_zp_y_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x10;
	mem[0x100] = 0x31; /* and ($20), y */
	mem[0x101] = 0x20;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8010] = 0xf0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.yr == 0x10);
	TEST_CHECK((cpu.sr) == 0);
	TEST_CHECK((cpu.ac) == 0x50);
}

void test_and_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x88;
	cpu.xr = 0x10;
	mem[0x20] = 0x80;
	mem[0x100] = 0x35; /* and $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == negative_flag);
	TEST_CHECK(cpu.ac == 0x80);
}


void test_rol_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x20] = 0x40;
	mem[0x100] = 0x36; /* rol $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = carry_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == negative_flag);
	TEST_CHECK(mem[0x20] == 0x81);
}

void test_sec(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x38; /* sec */
	cpu.pc = 0x100;
	cpu.sr = zero_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sr == (zero_flag | carry_flag));
}

void test_and_abs_y(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x39; /* and $1001,y */
	mem[0x101] = 0x01;
	mem[0x102] = 0x10;
	mem[0x1011] = 0x8f;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.yr = 0x10;
	cpu.ac = 0x80;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x103);
	TEST_CHECK((cpu.sr) == negative_flag);
	TEST_CHECK((cpu.ac) == 0x80);
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

void test_eor_zp_x_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x100] = 0x41; /* eor ($10, x) */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8000] = 0xf;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.xr == 0x10);
	TEST_CHECK((cpu.sr) == 0x0);
	TEST_CHECK((cpu.ac) == 0x5a);
}

void test_eor_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x18;
	mem[0x10] = 0x80;
	mem[0x100] = 0x45; /* eor $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == negative_flag);
	TEST_CHECK(cpu.ac == 0x98);
}

void test_lsr_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x10] = 0x81;
	mem[0x100] = 0x46; /* lsr $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == carry_flag);
	TEST_CHECK(mem[0x10] == 0x40);
}

void test_pha(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x48; /* pha */
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x80;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sp == 0xfe);
	TEST_CHECK(mem[0x1ff] == 0x80);
	TEST_CHECK(cpu.sr == 0x0);
}

void test_eor_imm(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x49; /* eor #80 */
	mem[0x101] = 0x80;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x88;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK((cpu.sr) == 0);
	TEST_CHECK((cpu.ac) == 0x8);
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

void test_eor_zp_y_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x10;
	mem[0x100] = 0x51; /* and ($20), y */
	mem[0x101] = 0x20;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8010] = 0xf0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.yr == 0x10);
	TEST_CHECK((cpu.sr) == negative_flag);
	TEST_CHECK((cpu.ac) == 0xa5);
}

void test_eor_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x88;
	cpu.xr = 0x10;
	mem[0x20] = 0x80;
	mem[0x100] = 0x55; /* eor $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == 0);
	TEST_CHECK(cpu.ac == 0x8);
}

void test_lsr_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x20] = 0x40;
	mem[0x100] = 0x56; /* lsr $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = carry_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == 0);
	TEST_CHECK(mem[0x20] == 0x20);
}

void test_cli(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x58; /* cli */
	cpu.pc = 0x100;
	cpu.sr = (negative_flag | interrupt_flag);
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sr == negative_flag);
}

void test_eor_abs_y(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x59; /* eor $1001,y */
	mem[0x101] = 0x01;
	mem[0x102] = 0x10;
	mem[0x1011] = 0xf;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.yr = 0x10;
	cpu.ac = 0x8f;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x103);
	TEST_CHECK((cpu.sr) == negative_flag);
	TEST_CHECK((cpu.ac) == 0x80);
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

void test_adc_zp_x_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x100] = 0x61; /* adc ($10, x) */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = carry_flag;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8000] = 0xf;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK((cpu.sr) == 0x0);
	TEST_CHECK((cpu.ac) == 0x65);
}

void test_adc_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x18;
	mem[0x10] = 0x80;
	mem[0x100] = 0x65; /* adc $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = carry_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == negative_flag);
	TEST_CHECK(cpu.ac == 0x99);
}

void test_ror_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x10] = 0x81;
	mem[0x100] = 0x66; /* ror $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = carry_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == (negative_flag | carry_flag));
	TEST_CHECK(mem[0x10] == 0xc0);
}

void test_pla(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x68; /* pla */
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.sp = 0xfe;
	mem[0x1ff] = 0x80;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sp == 0xff);
	TEST_CHECK(cpu.ac == 0x80);
	TEST_CHECK(cpu.sr == negative_flag);
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

void test_adc_zp_y_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x10;
	mem[0x100] = 0x71; /* adc ($20), y */
	mem[0x101] = 0x20;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8010] = 0xf0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.yr == 0x10);
	TEST_CHECK((cpu.sr) == carry_flag);
	TEST_CHECK((cpu.ac) == 0x45);
}

void test_adc_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x88;
	cpu.xr = 0x10;
	mem[0x20] = 0x80;
	mem[0x100] = 0x75; /* adc $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == (carry_flag | overflow_flag));
	TEST_CHECK(cpu.ac == 0x8);
}

void test_ror_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x20] = 0x0;
	mem[0x100] = 0x76; /* ror $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == zero_flag);
	TEST_CHECK(mem[0x20] == 0x0);
}

void test_sei(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x78; /* sei */
	cpu.pc = 0x100;
	cpu.sr = zero_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sr == (zero_flag | interrupt_flag));
}

void test_sta_zp_x_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x100] = 0x81; /* sta ($10, x) */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8000] = 0xf;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK((cpu.sr) == 0x0);
	TEST_CHECK(mem[0x8000] == 0x55);
}

void test_sty_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x5a;
	mem[0x100] = 0x84; /* sty $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK((cpu.sr) == 0x0);
	TEST_CHECK(mem[0x10] == 0x5a);
}

void test_sta_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x18;
	mem[0x100] = 0x85; /* sta $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == 0);
	TEST_CHECK(mem[0x10] == 0x18);
}

void test_stx_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x86; /* stx $10 */
	mem[0x101] = 0x10;
	cpu.xr = 0x81;
	cpu.pc = 0x100;
	cpu.sr = 0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == 0);
	TEST_CHECK(mem[0x10] == 0x81);
}

void test_dey(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x88; /* dey */
	cpu.pc = 0x100;
	cpu.sr = 0;
	cpu.yr = 0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sr == negative_flag);
	TEST_CHECK(cpu.yr == 0xff);
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

void test_sta_zp_y_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x10;
	mem[0x100] = 0x91; /* sta ($20), y */
	mem[0x101] = 0x20;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8010] = 0xf0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK((cpu.sr) == 0x0);
	TEST_CHECK(mem[0x8010] == 0x55);
}

void test_sty_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x5a;
	cpu.xr = 0x10;
	mem[0x100] = 0x94; /* sty $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK((cpu.sr) == 0x0);
	TEST_CHECK(mem[0x20] == 0x5a);
}

void test_sta_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x88;
	cpu.xr = 0x10;
	mem[0x100] = 0x95; /* sta $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == 0);
	TEST_CHECK(mem[0x20] == 0x88);
}

void test_stx_zp_y(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x10;
	mem[0x100] = 0x96; /* stx $10,y */
	mem[0x101] = 0x10;
	cpu.xr = 0x64;
	cpu.pc = 0x100;
	cpu.sr = 0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == 0);
	TEST_CHECK(mem[0x20] == 0x64);
}

void test_tya(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0x98; /* tya */
	cpu.pc = 0x100;
	cpu.sr = 0;
	cpu.yr = 0;
	cpu.ac = 0x80;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sr == zero_flag);
	TEST_CHECK(cpu.ac == 0x0);
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

void test_lda_zp_x_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x100] = 0xa1; /* lda ($10, x) */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8000] = 0xff;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK((cpu.sr) == negative_flag);
	TEST_CHECK(cpu.ac == 0xff);
}

void test_ldx_imm(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0xa2; /* ldx */
	mem[0x101] = 0x4e; /* #$4e */
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.xr == 0x4e);
	TEST_CHECK((cpu.sr) == 0);
	mem[0x100] = 0xa2; /* ldx */
	mem[0x101] = 0x00; /* #$0 */
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.xr == 0x0);
	TEST_CHECK((cpu.sr) == zero_flag);
	mem[0x100] = 0xa2; /* ldx */
	mem[0x101] = 0xff; /* #$ff */
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.xr == 0xff);
	TEST_CHECK((cpu.sr) == negative_flag);
}

void test_ldy_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x5a;
	mem[0x10] = 0x80;
	mem[0x100] = 0xa4; /* ldy $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == negative_flag);
	TEST_CHECK(cpu.yr == 0x80);
}

void test_lda_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x18;
	mem[0x10] = 0x80;
	mem[0x100] = 0xa5; /* lda $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == negative_flag);
	TEST_CHECK(cpu.ac == 0x80);
}

void test_ldx_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0xa6; /* ldx $10 */
	mem[0x101] = 0x10;
	mem[0x10] = 0x81;
	cpu.pc = 0x100;
	cpu.sr = 0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == negative_flag);
	TEST_CHECK(cpu.xr == 0x81);
}

void test_tay(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0xa8; /* tay */
	cpu.pc = 0x100;
	cpu.sr = 0;
	cpu.yr = 0;
	cpu.ac = 0x80;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sr == negative_flag);
	TEST_CHECK(cpu.yr == 0x80);
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

void test_lda_zp_y_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x10;
	mem[0x100] = 0xb1; /* lda ($20), y */
	mem[0x101] = 0x20;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8010] = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK((cpu.sr) == zero_flag);
	TEST_CHECK(cpu.ac == 0x0);
}

void test_ldy_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x5a;
	cpu.xr = 0x20;
	mem[0x30] = 0x0;
	mem[0x100] = 0xb4; /* ldy $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == zero_flag);
	TEST_CHECK(cpu.yr == 0x0);
}

void test_lda_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x88;
	cpu.xr = 0x10;
	mem[0x20] = 0x64;
	mem[0x100] = 0xb5; /* lda $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == 0);
	TEST_CHECK(cpu.ac == 0x64);
}

void test_ldx_zp_y(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x10;
	mem[0x20] = 0x64;
	mem[0x100] = 0xb6; /* ldx $10,y */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == 0);
	TEST_CHECK(cpu.xr == 0x64);
}

void test_clv(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0xb8; /* clv */
	cpu.pc = 0x100;
	cpu.sr = (negative_flag | overflow_flag);
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sr == negative_flag);
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

void test_cmp_zp_x_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x100] = 0xc1; /* cmp ($10, x) */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8000] = 0xff;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK((cpu.sr) == 0);
	TEST_CHECK(cpu.ac == 0x55);
}

void test_cpy_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x5a;
	mem[0x10] = 0x0;
	mem[0x100] = 0xc4; /* cpy $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == carry_flag);
	TEST_CHECK(cpu.yr == 0x5a);
}

void test_cmp_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x18;
	mem[0x10] = 0x80;
	mem[0x100] = 0xc5; /* cmp $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == negative_flag);
	TEST_CHECK(cpu.ac == 0x18);
}

void test_dec_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0xc6; /* dec $10 */
	mem[0x101] = 0x10;
	mem[0x10] = 0x0;
	cpu.pc = 0x100;
	cpu.sr = 0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == negative_flag);
	TEST_CHECK(mem[0x10] == 0xff);
}

void test_iny(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0xc8; /* iny */
	cpu.pc = 0x100;
	cpu.sr = 0;
	cpu.yr = 0x7f;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sr == negative_flag);
	TEST_CHECK(cpu.yr == 0x80);
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

void test_cmp_zp_y_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x10;
	mem[0x100] = 0xd1; /* cmp ($20), y */
	mem[0x101] = 0x20;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8010] = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK((cpu.sr) == carry_flag);
	TEST_CHECK(cpu.ac == 0x55);
}

void test_cmp_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x88;
	cpu.xr = 0x10;
	mem[0x20] = 0x64;
	mem[0x100] = 0xd5; /* cmp $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == carry_flag);
	TEST_CHECK(cpu.ac == 0x88);
}

void test_dec_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x20] = 0x1;
	mem[0x100] = 0xd6; /* dec $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == zero_flag);
	TEST_CHECK(mem[0x20] == 0x0);
}

void test_cld(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0xd8; /* clv */
	cpu.pc = 0x100;
	cpu.sr = (negative_flag | decimal_flag);
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sr == negative_flag);
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

void test_sbc_zp_x_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x100] = 0xe1; /* sbc ($10, x) */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = carry_flag;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8000] = 0xff;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK((cpu.sr) == 0);
	TEST_CHECK(cpu.ac == 0x56);
}

void test_cpx_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x20;
	mem[0x30] = 0x20;
	mem[0x100] = 0xe4; /* cpx $30 */
	mem[0x101] = 0x30;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == (carry_flag | zero_flag));
	TEST_CHECK(cpu.xr == 0x20);
}

void test_sbc_zp(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x18;
	mem[0x10] = 0x80;
	mem[0x100] = 0xe5; /* sbc $10 */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = carry_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == (negative_flag | overflow_flag));
	TEST_CHECK(cpu.ac == 0x98);
}

void test_inc_zp(void)
{
        CPU_CTX cpu;
        init_cpu(&cpu);
        mem[0x100] = 0xe6; /* inc $10 */
        mem[0x101] = 0x10;
        mem[0x10] = 0x7f;
        cpu.pc = 0x100;
        cpu.sr = 0;
        execute_instruction(&cpu);
        TEST_CHECK(cpu.pc == 0x102);
        TEST_CHECK(cpu.sr == negative_flag);
        TEST_CHECK(mem[0x10] == 0x80);
}    

void test_inx(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0xe8; /* inx */
	cpu.pc = 0x100;
	cpu.sr = 0;
	cpu.xr = 0xff;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sr == zero_flag);
	TEST_CHECK(cpu.yr == 0x0);
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

void test_sbc_zp_y_ind(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.yr = 0x10;
	mem[0x100] = 0xf1; /* sbc ($20), y */
	mem[0x101] = 0x20;
	cpu.pc = 0x100;
	cpu.sr = 0x0;
	cpu.ac = 0x55;
	mem[0x20] = 0x00;
	mem[0x21] = 0x80;
	mem[0x8010] = 0x0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK((cpu.sr) == carry_flag);
	TEST_CHECK(cpu.ac == 0x54);
}

void test_sbc_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.ac = 0x88;
	cpu.xr = 0x10;
	mem[0x20] = 0x64;
	mem[0x100] = 0xf5; /* sbc $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = carry_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == (carry_flag | overflow_flag));
	TEST_CHECK(cpu.ac == 0x24);
}

void test_inc_zp_x(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	cpu.xr = 0x10;
	mem[0x20] = 0xff;
	mem[0x100] = 0xf6; /* inc $10,x */
	mem[0x101] = 0x10;
	cpu.pc = 0x100;
	cpu.sr = 0;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x102);
	TEST_CHECK(cpu.sr == zero_flag);
	TEST_CHECK(mem[0x20] == 0x0);
}

void test_sed(void)
{
	CPU_CTX cpu;
	init_cpu(&cpu);
	mem[0x100] = 0xf8; /* sed */
	cpu.pc = 0x100;
	cpu.sr = negative_flag;
	execute_instruction(&cpu);
	TEST_CHECK(cpu.pc == 0x101);
	TEST_CHECK(cpu.sr == (negative_flag | decimal_flag));
}

TEST_LIST = {
	{"zero flag", test_zero_flag },
	{"negative flag", test_negative_flag },
	{"compare flags", test_cmp_flags },
	{"adc flags", test_adc_flags },
	{"adc bcd flags", test_adc_bcd_flags },
	{"sbc flags", test_sbc_flags },
	{"sbc bcd flags", test_sbc_bcd_flags },
	{"brk", test_brk}, 			// 0x0
	{"ora (zp,x)", test_ora_zp_x_ind},	// 0x1
	{"ora zp", test_ora_zp},		// 0x5
	{"asl zp", test_asl_zp},		// 0x6
	{"php", test_php}, 			// 0x8
	{"ora #", test_ora_imm}, 		// 0x9
	{"bpl", test_bpl},			// 0x10
	{"ora (zp),y", test_ora_zp_y_ind},	// 0x11
	{"ora zp,x", test_ora_zp_x},		// 0x15
	{"asl zp,x", test_asl_zp_x},		// 0x16
	{"clc", test_clc}, 			// 0x18
	{"ora abs,y", test_ora_abs_y}, 		// 0x19
	{"jsr", test_jsr},			// 0x20
	{"and (zp,x)", test_and_zp_x_ind},	// 0x21
	{"bit zp", test_bit_zp},		// 0x24
	{"and zp", test_and_zp},		// 0x25
	{"rol zp", test_rol_zp},		// 0x26
	{"plp", test_plp}, 			// 0x28
	{"and #", test_and_imm}, 		// 0x29
	{"bmi", test_bmi},			// 0x30
	{"and (zp),y", test_and_zp_y_ind},	// 0x31
	{"and zp,x", test_and_zp_x},		// 0x35
	{"rol zp,x", test_rol_zp_x},		// 0x36
	{"sec", test_sec}, 			// 0x38
	{"and abs,y", test_and_abs_y}, 		// 0x39
	{"rti", test_rti},			// 0x40
	{"eor (zp,x)", test_eor_zp_x_ind},	// 0x41
	{"eor zp", test_eor_zp},		// 0x45
	{"lsr zp", test_lsr_zp},		// 0x46
	{"pha", test_pha}, 			// 0x48
	{"eor #", test_eor_imm}, 		// 0x49
	{"bvc", test_bvc},			// 0x50
	{"eor (zp),y", test_eor_zp_y_ind},	// 0x51
	{"eor zp,x", test_eor_zp_x},		// 0x55
	{"lsr zp,x", test_lsr_zp_x},		// 0x56
	{"cli", test_cli}, 			// 0x58
	{"eor abs,y", test_eor_abs_y}, 		// 0x59
	{"rts", test_rts},			// 0x60
	{"adc (zp,x)", test_adc_zp_x_ind},	// 0x61	
	{"adc zp", test_adc_zp},		// 0x65
	{"ror zp", test_ror_zp},		// 0x66
	{"pla", test_pla}, 			// 0x68
	{"bvs", test_bvs},			// 0x70
	{"adc (zp),y", test_adc_zp_y_ind},	// 0x71
	{"adc zp,x", test_adc_zp_x},		// 0x75
	{"ror zp,x", test_ror_zp_x},		// 0x76
	{"sei", test_sei}, 			// 0x78
	{"sta (zp,x)", test_sta_zp_x_ind},	// 0x81
	{"sty zp", test_sty_zp},		// 0x84
	{"sta zp", test_sta_zp},		// 0x85
	{"stx zp", test_stx_zp},		// 0x86
	{"dey", test_dey}, 			// 0x88
	{"bcc", test_bcc},			// 0x90
	{"sta (zp),y", test_sta_zp_y_ind},	// 0x91
	{"sty zp,x", test_sty_zp_x},		// 0x94
	{"sta zp,x", test_sta_zp_x},		// 0x95
	{"stx zp,y", test_stx_zp_y},		// 0x96
	{"tya", test_tya}, 			// 0x98
	{"ldy #", test_ldy_imm},		// 0xa0
	{"lda (zp,x)", test_lda_zp_x_ind},	// 0xa1
	{"ldx #", test_ldx_imm},		// 0xa2
	{"ldy zp", test_ldy_zp},		// 0xa4
	{"lda zp", test_lda_zp},		// 0xa5
	{"ldx zp", test_ldx_zp},		// 0xa6
	{"tay", test_tay}, 			// 0xa8
	{"bcs", test_bcs},			// 0xb0
	{"lda (zp),y", test_lda_zp_y_ind},	// 0xb1
	{"ldy zp,x", test_ldy_zp_x},		// 0xb4
	{"lda zp,x", test_lda_zp_x},		// 0xb5
	{"ldx zp,y", test_ldx_zp_y},		// 0xb6
	{"clv", test_clv}, 			// 0xb8
	{"cpy #", test_cpy_imm},		// 0xc0
	{"cmp (zp,x)", test_cmp_zp_x_ind},	// 0xc1
	{"cpy zp", test_cpy_zp},		// 0xc4
	{"cmp zp", test_cmp_zp},		// 0xc5
	{"dec zp", test_dec_zp},		// 0xc6
	{"iny", test_iny}, 			// 0xc8
	{"bne", test_bne},			// 0xd0
	{"cmp (zp),y", test_cmp_zp_y_ind},	// 0xd1
	{"cmp zp,x", test_cmp_zp_x},		// 0xd5
	{"dec zp,x", test_dec_zp_x},		// 0xd6
	{"cld", test_cld}, 			// 0xd8
	{"cpx #", test_cpx_imm},		// 0xe0
	{"sbc (zp,x)", test_sbc_zp_x_ind},	// 0xe1
	{"cpx zp", test_cpx_zp},		// 0xe4
	{"sbc zp", test_sbc_zp},		// 0xe5
	{"inc zp", test_inc_zp},		// 0xe6
	{"inx", test_inx}, 			// 0xe8
	{"beq", test_beq},			// 0xf0
	{"sbc (zp),y", test_sbc_zp_y_ind},	// 0xf1
	{"sbc zp,x", test_sbc_zp_x},		// 0xf5
	{"inc zp,x", test_inc_zp_x},		// 0xf6
	{"sed", test_sed}, 			// 0xf8
	{ NULL, NULL }
};
