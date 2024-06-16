# Extended Lemon Microprocessor

This repository contains everything related to
the extended lemon microprocessor.

## C Library

The Extended Lemon Microprocessor is implemented as
a single header C library [extended_lemon.h](./extended_lemon.h).
You can just copy this file into your project and use it directly.
For more information read the source code.

## Specification

The Extended Lemon Microprocessor is defined in
the markdown document [extended_lemon.md](./extended_lemon.md).

## XLX

[xlx.c](./xlx.c) is a simple virtual machine powered by XL.
A good example of how to use the XL C library. This VM Can be
used to learn XL programming. Unfortunately there is no debug
functionality, but nobody stops you from implementing the debug
mode yourself.

You can store a byte to the address `$00FF` to write that byte to
the standard output. You can load the byte at the address `$00FF`
to read from the standard input.

To terminate the program you just need to store any value to
the address `$7FFF`.

### XLX File Format

XLX file is just 32KB of program memory (32768 bytes). XLX loads
those bytes to address `$8000` which is the second half of the XL
address space. The first half is the random access memory, the
second - read-only memory.

Why you cannot create a file less than 32KB? Because the last
8 bytes contain interrupt addresses, including of course the reset
interrupt address. That address is needed for the XL to found
the first instruction byte to execute.

### XLXDB

You can compile [xlx.c](./xlx.c) with `-DXLXDB` to get a version
of xlx that logs executed instructions.

## XLAS

[xlas.c](./xlas.c) is the XL assembler.
Do you really need an explanation?
See the examples!!!

## XLDIS

[xldis.c](./xldis.c) is the XL disassembler.
For this to work your binary file must be a valid `.xlx` file,
with all the instructions placed at the beginning of the file.
