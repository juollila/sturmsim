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

#include "cpu.h"

#ifndef KERNAL_H
#define KERNAL_H

typedef struct {
	u8 file;
	u8 device;
	u8 sa;
	u8 *name;
	bool open;
} FCB_CTX;

extern void init_kernal(void);
extern u8 kernal_setmsg(CPU_CTX *cpu);
extern u8 kernal_setlfs(CPU_CTX *cpu);
extern u8 kernal_open(CPU_CTX *cpu);
extern u8 kernal_clrchn(CPU_CTX *cpu);
extern u8 kernal_chrin(CPU_CTX *cpu);
extern u8 kernal_chrout(CPU_CTX *cpu);

#endif /* KERNAL_H */
