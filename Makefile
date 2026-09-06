CC = gcc
AS6502 = ../c/sturm6502/sturm6502

CFILES = sturmsim.c cpu.c
HFILES = sturmsim.h cpu.h

all:	sturmsim unittest rununittest

test:	test.asm
	$(AS6502) -o test.prg -l test.lst test.asm

unittest:	cpu.c cpu.h test/unittest.c
	$(CC) cpu.c test/unittest.c -o test/unittest

rununittest:	test/unittest
	test/unittest 

sturmsim:	$(CFILES) $(HFILES)
	$(CC) $(CFILES) -o sturmsim

clean:
	rm -f *.o sturmsim
	rm -f test.lst test.prg

