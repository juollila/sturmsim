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

void test_sbc_flags(void)
{
	CPU_CTX cpu;
	u8 result;
	init_cpu(&cpu);
	cpu.sr = carry_flag;
	result = sbc(&cpu, 0x0, 0x0);
	TEST_CHECK(result == 0x0);
	printf("%02x\n", result);
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

TEST_LIST = {
	{"zero flag", test_zero_flag },
	{"negative flag", test_negative_flag },
	{"compare flags", test_cmp_flags },
	{"adc flags", test_adc_flags },
	{"sbc flags", test_sbc_flags },
	{"brk", test_brk}, 			// 0x0
	{"ora (zp,x)", test_ora_zp_x_ind},	// 0x1
	{"ora zp", test_ora_zp},		// 0x5
	{"bpl", test_bpl},			// 0x10
	{"ora (zp),y", test_ora_zp_y_ind},	// 0x11
	{"ora zp,x", test_ora_zp_x},		// 0x15
	{"jsr", test_jsr},			// 0x20
	{"and (zp,x)", test_and_zp_x_ind},	// 0x21
	{"bit zp", test_bit_zp},		// 0x24
	{"and zp", test_and_zp},		// 0x25
	{"bmi", test_bmi},			// 0x30
	{"and (zp),y", test_and_zp_y_ind},	// 0x31
	{"and zp,x", test_and_zp_x},		// 0x35
	{"rti", test_rti},			// 0x40
	{"eor (zp,x)", test_eor_zp_x_ind},	// 0x41
	{"eor zp", test_eor_zp},		// 0x45
	{"bvc", test_bvc},			// 0x50
	{"eor (zp),y", test_eor_zp_y_ind},	// 0x51
	{"eor zp,x", test_eor_zp_x},		// 0x55
	{"rts", test_rts},			// 0x60
	{"adc (zp,x)", test_adc_zp_x_ind},	// 0x61	
	{"bvs", test_bvs},			// 0x70
	{"adc (zp),y", test_adc_zp_y_ind},	// 0x71
	{"sta (zp,x)", test_sta_zp_x_ind},	// 0x81
	{"sty zp", test_sty_zp},		// 0x84
	{"bcc", test_bcc},			// 0x90
	{"sta (zp),y", test_sta_zp_y_ind},	// 0x91
	{"sty zp,x", test_sty_zp_x},		// 0x94
	{"ldy #", test_ldy_imm},		// 0xa0
	{"lda (zp,x)", test_lda_zp_x_ind},	// 0xa1
	{"ldx #", test_ldx_imm},		// 0xa2
	{"ldy zp", test_ldy_zp},		// 0xa4
	{"bcs", test_bcs},			// 0xb0
	{"lda (zp),y", test_lda_zp_y_ind},	// 0xb1
	{"ldy zp,x", test_ldy_zp_x},		// 0xb4
	{"cpy #", test_cpy_imm},		// 0xc0
	{"cmp (zp,x)", test_cmp_zp_x_ind},	// 0xc1
	{"cpy zp", test_cpy_zp},		// 0xc4
	{"bne", test_bne},			// 0xd0
	{"cmp (zp),y", test_cmp_zp_y_ind},	// 0xd1
	{"cpx #", test_cpx_imm},		// 0xe0
	{"sbc (zp,x)", test_sbc_zp_x_ind},	// 0xe1
	{"cpx zp", test_cpx_zp},		// 0xe4
	{"beq", test_beq},			// 0xf0
	{"sbc (zp),y", test_sbc_zp_y_ind},	// 0xf1
	{ NULL, NULL }
};
