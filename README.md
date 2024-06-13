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

XLX is a simple virtual machine powered by XL. A good example of how
to use the XL C library. This VM Can be used to learn XL programming.
Unfortunately there is no debug functionality, but nobody stops you
from implementing the debug mode yourself.

### XLX File Format

XLX file is just 32KB of program memory (32768 bytes). XLX loads
those bytes to address `$8000` which is the second half of the XL
address space. The first half is the random access memory, the
second - read-only memory.

Why you cannot create a file then 32KB? Because the last 8 bytes
contain interrupt addresses, including of course the reset interrupt
address. That address is needed for the XL to found the first
instruction byte to execute.
