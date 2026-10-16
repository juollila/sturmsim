INTRODUCTION
============

SturmSim is a small 6502 CPU simulator and command-line unit test runner for
SturmMon (https://github.com/juollila/sturmmon). It loads a Commodore 64 PRG
file, initializes the simulated CPU and memory, and executes the program
without requiring a full C64 emulator. This makes SturmMon's unit tests easy
to run from a shell or a Makefile instead of loading them manually in an
emulator.

USAGE
=====

	sturmsim [options] <file.prg>

Options:

	--execution-address=<value>	Set the execution start address.
	--help				Display this message.
	--lower-case			Enable lower-case PETSCII output.

The default execution address is taken from the start address stored at the
beginning of the PRG file.

Example:

	sturmsim --execution-address=0810 test.prg

KERNAL ROUTINES
===============

The simulator includes a minimal C64 KERNAL emulation layer for routines used
by the test programs, such as CHRIN, CHROUT, SETLFS, OPEN, CHKOUT and PLOT.

BRK VECTOR
==========

The default BRK vector is initialized to the C64 ROM vector at $316/$317
($FE66), and the simulator halts execution when that default BRK path is hit.
If a different vector is installed, execution continues by jumping to it.