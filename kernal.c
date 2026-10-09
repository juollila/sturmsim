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

static u8 input_device = 0;
static u8 output_device = 3;
static u8 io_status = 0;

static u8 lfs_file, lfs_device, lfs_sa;

FCB_CTX fcb[MAX_FILES];

void init_kernal(void)
{
	for (int i = 0; i < MAX_FILES ; i++) fcb[i].open = false;
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

u8 kernal_setmsg(CPU_CTX *cpu)
{
	cpu->sr &= (~carry_flag);
	return RTS;
}


u8 kernal_readst(CPU_CTX *cpu)
{
	cpu->ac = io_status;
	return RTS;
}

u8 kernal_setlfs(CPU_CTX *cpu)
{
	lfs_file = cpu->ac;
	lfs_device = cpu->xr;
	lfs_sa = cpu->yr;
	cpu->sr &= (~carry_flag);
	return RTS;
}

u8 kernal_open(CPU_CTX *cpu)
{
	if (find_fcb(lfs_file)) {
		fprintf(stderr, "FILE OPEN ERROR\n");
		cpu->sr |= carry_flag;
		return RTS;
	}
	FCB_CTX *new = find_free_fcb();
	if (new == NULL) {
		fprintf(stderr, "TOO MANY FILES ERROR\n");
		cpu->sr |= carry_flag;
		return RTS;
	}
	new->file = lfs_file;
	new->device = lfs_device;
	new->sa = lfs_sa;
	new->open = true;
	//printf("kernal_open: %d, %d, %d\n", new->file, new->device, new->sa);
	cpu->sr &= (~carry_flag);
	return RTS;
}

u8 kernal_chkout(CPU_CTX *cpu)
{
	u8 file = cpu->xr;
	//printf("kernal_chkout: %d\n", file);
	FCB_CTX *fcb = find_fcb(file);
	if (fcb == NULL || fcb->open == false) {
		fprintf(stderr, "FILE_NOT_OPEN\n");
		cpu->sr |= carry_flag;
		cpu->ac = 0x3;
		return RTS;
	}
	if (fcb->device == 0x0) {
		fprintf(stderr, "NOT_OUTPUT_FILE\n");
		cpu->sr |= carry_flag;
		cpu->ac = 0x0;
		return RTS;
	}
	if (fcb->device == 0x3) {
		output_device = 0x3;
		cpu->sr &= (~carry_flag);
		return RTS;
	}
	fprintf(stderr, "DEVICE_NOT_PRESENT\n");
	cpu->sr |= carry_flag;
	cpu->ac = 0x5;
	return RTS;
}

u8 kernal_clrchn(CPU_CTX *cpu)
{
	input_device = 0;
	output_device = 3;
	cpu->sr &= (~carry_flag);
	return RTS;
}

u8 kernal_chrin(CPU_CTX *cpu, bool lower)
{
	u8 c, i;
	u8 len = mem[KBLEN];
	if (len == 0) {
		struct termios term_old, term_new;
		tcgetattr(STDIN_FILENO, &term_old);
		term_new = term_old;
		term_new.c_lflag &= ~(ICANON | ECHO); /* disable buffering and echo*/
		tcsetattr(STDIN_FILENO, TCSANOW, &term_new);
		c = getchar();
		tcsetattr(STDIN_FILENO, TCSANOW, &term_old);
	} else {
		c = mem[KEYBOARD_BUFFER];
		for (i = 0; i < len; i++)
			mem[KEYBOARD_BUFFER + i] = mem[KEYBOARD_BUFFER + i + 1];
		mem[KBLEN] = len - 1;
	}
	if (lower == false && c >= 0x61 && c <= 0x7a)
		c -= 0x20;
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
	if (lower == false && i >= 0x60) i = '.';
	if (lower == true) {
		if (i>=0x41 && i<=0x5a)
			i += 0x20;
		else if (i>=0x61 && i<=0x7a)
			i -= 0x20;
		else if (i>=0xc1 && i<=0xda)
			i &= 0x7f;
	}
	if (lower == true && i >= 0x80) i = '.';
	putchar(i);
	cpu->sr &= (~carry_flag);
	return RTS;
}

u8 kernal_plot(CPU_CTX *cpu)
{
	int x, y;
	if ((cpu->sr & carry_flag) == carry_flag) {
		struct termios term_old, term_new;
		tcgetattr(STDIN_FILENO, &term_old);
		term_new = term_old;
		term_new.c_lflag &= ~(ICANON | ECHO); /* disable buffering and echo*/
		tcsetattr(STDIN_FILENO, TCSANOW, &term_new);
		printf("\033[6n");
		fflush(stdout);
		scanf("\033[%d;%dR", &x, &y);
		cpu->xr = (u8) x;
		cpu->yr = (u8) y;
		tcsetattr(STDIN_FILENO, TCSANOW, &term_old);
	} else {
		printf("\033[%d;%dH", cpu->xr, cpu->yr);
	}
	return RTS;
}
