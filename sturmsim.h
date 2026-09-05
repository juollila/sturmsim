typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef struct {
	char *program_file_name;
} CTX;

typedef struct {
	u8 a;
	u8 x;
	u8 y;
	u8 sp;
	u8 st;
	u16 pc;
	bool halt;
} CPU_CTX;

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

const u8 carry_flag = 0x1;
const u8 zero_flag = 0x2;
const u8 interrupt_flag = 0x4;
const u8 decimal_flag = 0x8;
const u8 break_flag = 0x10;
const u8 one_flag = 0x20;
const u8 overflow_flag = 0x40;
const u8 negative_flag = 0x80;

