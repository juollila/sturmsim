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

void init_cpu(CPU_CTX *cpu);
void init_memory(void);
static u8 fetch_byte(CPU_CTX *cpu);
static u8 peek_byte(CPU_CTX *cpu);
static u16 fetch_word(CPU_CTX *cpu);
static u16 peek_word(CPU_CTX *cpu);
static void push_byte(CPU_CTX *cpu, u8 i);
static void push_word(CPU_CTX *cpu, u16 i);
static u8 pull_byte(CPU_CTX *cpu);
static u16 pull_word(CPU_CTX *cpu);
static void chrout(u8 i);
static void set_nz_flags(CPU_CTX *cpu, u8 i);
static void print_registers(CPU_CTX *cpu);
void execute_instruction(CPU_CTX *cpu);
void execute_program(CPU_CTX *cpu);

#endif /* CPU_H */
