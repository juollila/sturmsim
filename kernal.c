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
#include <unistd.h>
#include <termios.h>

#include "cpu.h"
#include "kernal.h"

#define RTS 0x60
#define MAX_FILES 10

#define KEYBOARD_BUFFER 0x277
#define KBLEN 0xc6

#undef DEBUG

static u8 input_device = 0;
static u8 output_device = 3;
static u8 io_status = 0;

static u8 lfs_file, lfs_device, lfs_sa;

static u8 name[256];

static FCB_CTX fcb[MAX_FILES];

static struct termios term_old, term_new;

static void disable_echo()
{
	tcgetattr(STDIN_FILENO, &term_old);
	term_new = term_old;
	term_new.c_lflag &= ~(ICANON | ECHO); /* disable buffering and echo*/
	tcsetattr(STDIN_FILENO, TCSANOW, &term_new);
}

static void enable_echo()
{
	tcsetattr(STDIN_FILENO, TCSANOW, &term_old);
}

static FCB_CTX *find_free_fcb(void)
{
	for (int i = 0; i < MAX_FILES; i++) {
		if (fcb[i].open == false) return &fcb[i];
	}
	return NULL;
}

static FCB_CTX *find_fcb(int file)
{
	for (int i = 0; i < MAX_FILES; i++) {
		if (fcb[i].file == file) return &fcb[i];
	}
	return NULL;
}

static u8 ascii2pet(u8 i, bool lower)
{
	if (lower == true && i >= 0x41 && i <= 0x5a) i+= 0x80;
	if (i >= 0x61 && i <= 0x7a) i -= 0x20;
	return i;
}

static u8 pet2ascii(u8 i, bool lower)
{
	if (lower == true) {
		if (i>=0x41 && i<=0x5a)
			i += 0x20;
		else if (i>=0x61 && i<=0x7a)
			i -= 0x20;
		else if (i>=0xc1 && i<=0xda)
			i &= 0x7f;
	}
	return i;
}

void init_kernal(void)
{
	for (int i = 0; i < MAX_FILES ; i++) fcb[i].open = false;
}

u8 kernal_setmsg(CPU_CTX *cpu)
{
	cpu->sr &= (~carry_flag);
#ifdef DEBUG
	printf("kernal_setmsg\n");
#endif
	return RTS;
}


u8 kernal_readst(CPU_CTX *cpu)
{
	cpu->ac = io_status;
#ifdef DEBUG
	printf("kernal_readst\n");
#endif
	return RTS;
}

u8 kernal_setlfs(CPU_CTX *cpu)
{
	lfs_file = cpu->ac;
	lfs_device = cpu->xr;
	lfs_sa = cpu->yr;
	cpu->sr &= (~carry_flag);
#ifdef DEBUG
	printf("kernal_setlfs: %d %d %d\n", lfs_file, lfs_device, lfs_sa); 
#endif
	return RTS;
}

u8 kernal_setnam(CPU_CTX *cpu, bool lower)
{
	u8 i;
	u16 address = cpu->yr * 0x100 + cpu->xr;
	u8 length = cpu->ac;
	cpu->sr &= (~carry_flag);
	if (length == 0)
		return RTS;
	for(i = 0; i < length; i ++) {
		name[i] = pet2ascii(mem[address + i], lower); 
	}
	name[i] = 0;
#ifdef DEBUG
	printf("kernal_setname: %s\n", name);
#endif
	return RTS;
}

u8 kernal_open(CPU_CTX *cpu)
{
	if (find_fcb(lfs_file)) {
		fprintf(stderr, "\nFILE OPEN ERROR\n");
		cpu->sr |= carry_flag;
		return RTS;
	}
	FCB_CTX *new = find_free_fcb();
	if (new == NULL) {
		fprintf(stderr, "\nTOO MANY FILES ERROR\n");
		cpu->sr |= carry_flag;
		return RTS;
	}
	new->file = lfs_file;
	new->device = lfs_device;
	new->sa = lfs_sa;
	new->open = true;
#ifdef DEBUG
	printf("kernal_open: %d, %d, %d\n", new->file, new->device, new->sa);
#endif
	cpu->sr &= (~carry_flag);
	return RTS;
}

u8 kernal_close(CPU_CTX *cpu)
{
	FCB_CTX *fcb = find_fcb(cpu->ac);
	if (fcb) fcb->open = false;
#ifdef DEBUG
	printf("kernal_close: %d\n", cpu->ac);
#endif
	cpu->sr &= (~carry_flag);
	return RTS;
}

u8 kernal_chkin(CPU_CTX *cpu) {
	u8 file = cpu->xr;
	FCB_CTX *fcb = find_fcb(file);
#ifdef DEBUG
	printf("kernal_checkin: %d\n", file);
#endif
	if (fcb == NULL || fcb->open == false) {
		fprintf(stderr, "\nFILE_NOT_OPEN\n");
		cpu->sr |= carry_flag;
		cpu->ac = 0x3;
		return RTS;
	}
	if (fcb->device == 0x0) {
		input_device = 0x0;
		cpu->sr &= (~carry_flag);
		return RTS;
	}
	if (fcb->device == 0x3) {
		input_device = 0x3;
		cpu->sr &= (~carry_flag);
		return RTS;
	}
	fprintf(stderr, "\nDEVICE_NOT_PRESENT\n");
	cpu->sr |= carry_flag;
	cpu->ac = 0x5;
	return RTS;
}

u8 kernal_chkout(CPU_CTX *cpu)
{
	u8 file = cpu->xr;
#ifdef DEBUG
	printf("kernal_checkout: %d\n", file);
#endif
	FCB_CTX *fcb = find_fcb(file);
	if (fcb == NULL || fcb->open == false) {
		fprintf(stderr, "\nFILE_NOT_OPEN\n");
		cpu->sr |= carry_flag;
		cpu->ac = 0x3;
		return RTS;
	}
	if (fcb->device == 0x0) {
		fprintf(stderr, "\nNOT_OUTPUT_FILE\n");
		cpu->sr |= carry_flag;
		cpu->ac = 0x0;
		return RTS;
	}
	if (fcb->device == 0x3) {
		output_device = 0x3;
		cpu->sr &= (~carry_flag);
		return RTS;
	}
	fprintf(stderr, "\nDEVICE_NOT_PRESENT\n");
	cpu->sr |= carry_flag;
	cpu->ac = 0x5;
	return RTS;
}

u8 kernal_clrchn(CPU_CTX *cpu)
{
	input_device = 0;
	output_device = 3;
	cpu->sr &= (~carry_flag);
#ifdef DEBUG
	printf("kernal_clrchn\n");
#endif
	return RTS;
}

u8 kernal_chrin(CPU_CTX *cpu, bool lower)
{
	int c;
	u8 i;
	u8 len = mem[KBLEN];
	if (len == 0) {
		bool terminal_input = isatty(STDIN_FILENO);
		if (terminal_input) disable_echo();
		c = getchar();
		if (terminal_input) enable_echo();
		if (c == EOF) {
			cpu->halt = true;
			return RTS;
		}
	} else {
		c = mem[KEYBOARD_BUFFER];
		for (i = 0; i < len; i++)
			mem[KEYBOARD_BUFFER + i] = mem[KEYBOARD_BUFFER + i + 1];
		mem[KBLEN] = len - 1;
	}
	/* while (c == 0x7f || c == '\b' ) {
 		printf("\b \b");
		fflush(stdout);
		disable_echo();
		c = getchar();
		enable_echo();
	} */
	c = ascii2pet(c, lower);
	if (c >= ' ')
		putchar(c);
	if (c == '\n')
		c = '\r';
	cpu->ac = c;
	cpu->sr &= (~carry_flag);
	return RTS;
}


u8 kernal_chrout(CPU_CTX *cpu, bool lower)
{
	u8 i = cpu->ac;
	/* ignore '\n' */
	if (i == 0xa) {
		cpu->sr &= (~carry_flag);
		return RTS;
	}
	if (i == 0xd) i = '\n';
	/* reverse on */
	if (i == 0x12) {
		printf("\033[7m");
		cpu->sr &= (~carry_flag);
		return RTS;
	}
	/* reverse off */
	if (i == 0x92) {
		printf("\033[27m");
		cpu->sr &= (~carry_flag);
		return RTS;
	}
	/* shift + space */
	if (i == 0xa0)
		i = 0x20;
	i = pet2ascii(i, lower);
	if (i >= 0x7f) i = '.';
	if (i <= 0x20 & i!='\n') i = '.';
	putchar(i);
	cpu->sr &= (~carry_flag);
	return RTS;
}

u8 kernal_load(CPU_CTX *cpu)
{
	u16 address;
	int ch, low, high;

	if (cpu->ac != 0) {
		fprintf(stderr, "\nTODO: VERIFY\n");
		cpu->sr |= carry_flag;
		return RTS;
	}
	if (lfs_sa == 0) {
		fprintf(stderr, "\nTODO: IGNORE HEADER\n");
		cpu->sr |= carry_flag;
		return RTS;
	}

	FILE *program = fopen((const char *)&name[0], "rb");
	if (program == NULL) {
		fprintf(stderr, "\nFILE_NOT_FOUND_ERROR\n");
		cpu->sr |= carry_flag;
		return RTS;
	}

	low = fgetc(program);
	high = fgetc(program);

	if (low != EOF && high != EOF) {
		address = (high * 0x100) + low;
		while (( ch = fgetc(program)) != EOF) {
			mem[address++] = (u8) ch;
		}
	} else {
		fclose(program);
		fprintf(stderr, "\nLOAD_ERROR\n");
		cpu->sr |= carry_flag;
		return RTS;
	}
	fclose(program);
#ifdef DEBUG
	printf("kernal_load\n");
#endif
	cpu->sr &= (~carry_flag);
	return RTS;
}

u8 kernal_save(CPU_CTX *cpu)
{
	u16 start_address, end_address;
	int i, res1, res2;

	if (lfs_device == 0 || lfs_device == 2 || lfs_device == 3) {
		fprintf(stderr, "\nILLEGAL_DEVICE_NUMBER\n");
		cpu->sr |= carry_flag;
		return RTS;
	}
	if (lfs_device == 1) {
		fprintf(stderr, "\nTODO: SAVE TO DATASETTE\n");
		cpu->sr |= carry_flag;
		return RTS;
	}

	FILE *program = fopen((const char *)&name[0], "wb");
	if (program == NULL) {
		fprintf(stderr, "\nCANNOT_OPEN_FILE_ERROR\n");
		cpu->sr |= carry_flag;
		return RTS;
	}
	start_address = (mem[cpu->ac + 1] * 0x100) + mem[cpu->ac];
	end_address = (cpu->yr * 0x100) + cpu->xr;

	res1 = fputc(start_address & 0xff, program);
	res2 = fputc(start_address >> 8, program);
	if (res1 != EOF && res2 != EOF) {
		for (i = start_address; i < end_address ; i++) {
			res1 = fputc(mem[i], program);
			if (res1 == EOF) {
				fclose(program);
				fprintf(stderr, "\nSAVE_ERROR\n");
				cpu->sr |= carry_flag;
				return RTS;
			}
		}

	} else {
		fclose(program);
		fprintf(stderr, "\nSAVE_ERROR\n");
		cpu->sr |= carry_flag;
		return RTS;
	}
	fclose(program);
#ifdef DEBUG
	printf("kernal_save\n");
#endif
	cpu->sr &= (~carry_flag);
	return RTS;
}

u8 kernal_plot(CPU_CTX *cpu)
{
	int x, y;
	if ((cpu->sr & carry_flag) == carry_flag) {
		/* return if stdin is not terminal */
		if (isatty(STDIN_FILENO) == 0)
			return RTS;
		disable_echo();
		printf("\033[6n");
		fflush(stdout);
		scanf("\033[%d;%dR", &x, &y);
		cpu->xr = (u8) x;
		cpu->yr = (u8) y;
		enable_echo();
	} else {
		/* return if stdout is not terminal */
		if (isatty(STDOUT_FILENO) == 0)
			return RTS;
		printf("\033[%d;%dH", cpu->xr, cpu->yr);
	}
	return RTS;
}
