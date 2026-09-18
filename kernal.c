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

u8 kernal_setmsg(CPU_CTX *cpu)
{
	cpu->sr &= (~carry_flag);
	return RTS;
}

u8 kernal_chrin(CPU_CTX *cpu)
{
	struct termios term_old, term_new;
	tcgetattr(STDIN_FILENO, &term_old);
	term_new = term_old;
	term_new.c_lflag &= ~(ICANON | ECHO); /* disable buffering and echo*/
	tcsetattr(STDIN_FILENO, TCSANOW, &term_new);
	char c = getchar();
	if (c >= ' ')
		putchar(c);
	if (c == '\n')
		c = '\r';
	cpu->ac = (u8) c;
	tcsetattr(STDIN_FILENO, TCSANOW, &term_old);
	cpu->sr &= (~carry_flag);
	return RTS;
}


u8 kernal_chrout(CPU_CTX *cpu)
{
	u8 i = cpu->ac;
	if (i == 0xd) i = '\n';
	if (i >= 0x60) i = '.'; 
	putchar(i);
	cpu->sr &= (~carry_flag);
	return RTS;
}

