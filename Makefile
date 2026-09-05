CC = gcc
AS6502 = ../c/sturm6502/sturm6502

CFILES = sturmsim.c cpu.c
HFILES = sturmsim.h cpu.h

all:	sturmsim test

test:	test.asm
	$(AS6502) -o test.prg -l test.lst test.asm

sturmsim:	$(CFILES) $(HFILES)
	$(CC) $(CFILES) -o sturmsim

clean:
	rm -f *.o sturmsim
	rm -f test.lst test.prg

