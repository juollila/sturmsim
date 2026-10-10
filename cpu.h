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

#include <stdbool.h>
#include "sturmsim.h"

#ifndef CPU_H
#define CPU_H

#define MEM_SIZE 65536

typedef unsigned char u8;
typedef signed char i8;
typedef unsigned short u16;
typedef signed short i16;
typedef unsigned int u32;

typedef struct {
	u16 pc;
	u8 sr;
	u8 ac;
	u8 xr;
	u8 yr;
	u8 sp;
	bool halt;
} CPU_CTX;

extern u8 mem[];

/************************************************
 * status register flags
 * 76543210
 * NV1BDIZC
 * C = carry
 * Z = zero
 * I = interrupt disable
 * D = decimal
 * B = break
 * 1 = always 1
 * V = overflow
 * N = negative
 ************************************************/

static const u8 carry_flag = 0x1;
static const u8 zero_flag = 0x2;
static const u8 interrupt_flag = 0x4;
static const u8 decimal_flag = 0x8;
static const u8 break_flag = 0x10;
static const u8 one_flag = 0x20;
static const u8 overflow_flag = 0x40;
static const u8 negative_flag = 0x80;

void init_cpu(CPU_CTX *cpu);
void init_memory(void);
void set_nz_flags(CPU_CTX *cpu, u8 i);
void set_cmp_flags(CPU_CTX *cpu, u8 i, u8 j);
u8 adc(CPU_CTX *cpu, u8 i, u8 j);
u8 sbc(CPU_CTX *cpu, u8 i, u8 j);
void execute_instruction(SIM_CTX *sim, CPU_CTX *cpu);
void execute_program(SIM_CTX *sim, CPU_CTX *cpu);

#endif /* CPU_H */
