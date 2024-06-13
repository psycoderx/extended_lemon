# Extended Lemon Standard

## About This Document

This document defines the behavior of
The Extended Lemon Microprocessor.

## Convention

Hexadecimal numbers are prefixed with a `$` sign.

Binary numbers are prefixed with a `%` sign.

Bit is iether 0 or 1.

Byte is 8 bits.

The bits are enumerated from the least significant bit
to the most significant bit. For example in the number
`%00000010` the 2nd bit is set.

Memory is a 1D array of bytes each with its own address.

Bus is a comunication system that transfers data between
CPU and other devices.

Register is a small internal CPU memory with
a predefined role.

Interrupt is a request (or a signal) for CPU to do
something special immediately.

Instruction is a precise information that tells CPU what
exactly to do.

Addressing mode is a method for loading or/and computing
an address.

Cycle is the minimum unit of time during which CPU performs
a minimum part of the work.

The address of a multibyte object is the address of
the first byte of that object.

LSB stands for the least significant byte.

MSB stands for the most significant byte.

## Introduction

The Extended Lemon Microprocessor (or The XL) is an 8 bit
processor with 16 bit address bus. Addresses are in range
from `$0000` to `$FFFF`.

The XL is not intended to be implemented as a phisical device.

The XL is little endian, which means addresses are stored
in memory least significant byte first. For example
the address `$ABCD` is stored as two bytes `$CD`, `$AB`.

The zero page is a range of addresses from `$0000` to `$00FF`
(the fisrt 256 bytes).
The stack is a range of addresses from `$0100` to `$01FF`
(the second 256 bytes).
The interrupt addresses are stored in range `$FFF8-$FFFF`
(the last 8 bytes).

## Registers

The XL has 6 registers. However the implementation
may use additional registers as long as the behavior
of the implementation is compatible with the XL standard.

### A (Accumulator)
The accumulator is an 8-bit register which is used as
an argument or as a place for the result of an instruction.
This is the only register for which all kinds of operations
are possible: load, store, push, pull, transfer, bitwise,
arithmetic, and compare operations.

### F (Status Flags)
The status flags is an 8 bit register which contains
an information collected when executing instructions.
Each bit of this register has its own meaning.
Some bits are used to control the behavior of the XL.
The status flags can be pushed, pulled, and modified
by instructions and interrupts.

### P (Program Counter)
The program counter is a 16 bit register which contains
the address of the next instruction to be executed.
When the XL executes an instruction the program counter
may contain the address of the instruction argument.
The value of this register can be modified by executing
a jump, a conditional jump or a subroutine call to another
address or by returning from a subroutine or an interrupt.

### S (Stack Index)
The stack index is an 8 bit register which contains
the index that is used to access the stack for push and
pull operations. This register points to the next free
location on the stack. When one byte is pushed onto
the stack the stack index is decremented by 1, and
incremented by 1 when the byte pulled back from the stack.

### X and Y (Index Registers)
X and Y are 8 bit registers. X and Y are used as offsets
when accessing memory or as counters. Both X and Y can be
loaded, stored, pushed, pulled, compared, incremented,
and decremented by instructions.

## Bits of the Status Flags

### B (BRK Flag)
The BRK flag is the 1st bit of the status flags.
This flag is set by the `BRK` instruction and only used
to determine what coused the `BREAK` interrupt.

### C (Carry Flag)
The carry flag is the 2nd bit of the status flags.
This flag is set to 1 if the result computed by
the last instruction is exceeded the range of one byte,
this flag is set to 0 if the result is not exceeded
the range. The value of this flag is not affected by
instructions that have no result.

### D (Disable Breaks Flag)
The disable breaks flag is the 3rd bit of the status flags.
This flag is set by the program. When this flag is 1 all
`BREAK` interrupts are ignored.

### N (Negative Flag)
The negative flag is the 4th bit of the status flags.
This flag equals to the 8th bit of the result computed
by the last instruction. The value of this flag is not
affected by instructions that have no result.

### R (Reserved Flag)
The reserved flag is the 5th bit of the status flags.
This flag is reserved for future updates and should not
be used by the implementation or the program.
The value of this flag is always 0.

### U (User Defined Flag)
The user defined flag is the 6th bit of the status flags.
This flag can be used by the program and should not be used
by the implementation.

### V (Overflow Flag)
The overflow flag is the 7th bit of the status flags.
This flag is set to 1 if a sum of two positive numbers is
negative, or if a sum of two negative numbers is positive,
for any other sum this flag is set to 0. The value of this
flag is only affected by arithmetic and compare operations.

### Z (Zero Flag)
The zero flag is the 8th bit of the status flags.
This flag is set to 1 if the result computed by the last
instruction is zero, and this flag is set to 0 if the result
is not zero. The value of this flag is not affected by
instructions that have no result.

## Cycle

When running the XL repeat the next sequence in a loop:

1. Process interrupts if any.
2. Read the instruction byte at the program counter.
3. Increment the program counter by 1.
4. Run the adressing mode according to the instruction byte.
5. Run the instruction according to the instruction byte.

How many cycles it will take for the XL to finnish the above
sequence depends on interrupts if requested, the instruction
and the addressing mode.

The minimum number of cycles is 1, which can only happen
when there are no interrupts requested, no addressing mode is
executed and the instruction takes 1 cycle to finnish.

## Interrupts

There are 3 different interrupts in the XL: `RESET`, `REACT`,
and `BREAK`. Each interrupt has its own address that points
to the corresponding interrupt handler. The last 8 bytes of
memory are used to store these addresses, 2 bytes of which
are reserved.

If something requests an interrupt, the XL will finish
the instruction it's currently executing and only then it
will respond to the interrupt. If several interrupts are
requested before the instruction is finished, only the one
with the highest piority is actually processed.

### RESET
The reset interrupt is used to request a complete
re-initialization of the XL. It sets all 6 registers to 0
and jumps to the corresponding interrupt handler. The state
of the XL is not saved on the stack so returning from
the interrupt handler is impossible. The XL automatically
requests the reset interrupt before executing
the first instruction.
The priority of this interrupt is 6, the highest of all.
The address of the interrupt handler is located at
`$FFFE` (LSB) and `$FFFF` (MSB). It takes 2 cycles for
the XL to process this interrupt.

### REACT
The react interrupt can only be requested by an external
signal and the request can only be ignored due to priority.
The priority of this interrupt is 4.
The address of the interrupt handler is located at
`$FFFC` (LSB) and `$FFFD` (MSB). It takes 5 cycles for
the XL to process this interrupt.

### BREAK
The break interrupt can be requested by an external signal or
a `BRK` instruction. When this interrupt is requested and not
ignored due to priority, at the end of
the current instruction execution the request will not be
processed if the disable breaks flag is set to 1.
The priority of this interrupt is 2, the lowest of all.
The address of the interrupt handler is located at
`$FFFA` (LSB) and `$FFFB` (MSB). It takes 5 cycles for
the XL to process this interrupt.

### REACT or BREAK
When a react or break interrupt is processed the XL will:
1. Push MSB of the program counter.
2. Push LSB of the program counter.
3. Push the status flags.
4. Set the disable breaks flag to 1.
5. If this interrupt is requested by the `BRK` instruction,
   set the BRK flag to 1.
8. Jump to the corresponding interrupt handler.

### Reserved Interrupt
The bytes at `$FFF8` and `$FFF9` are reserved and they
should not be used by the implementation and the program.

## Addressing Modes

The instruction byte is the first byte of the instruction.
The address obtained by an addressing mode is called the `addr`.

There are 12 addressing modes: NAM, IMM, ABS, ABX, ABY, REL, ZPG,
ZPX, ZPY, VEC, ZVX, and ZYV.

### NAM (No addressing mode)
The `addr` is unused.

### IMM (Immediate)
The `addr` points to the second byte of the instruction.
The instruction size is 2 bytes.

### ABS (Absolute)
The second byte of the instruction specifies the LSB of
the `addr`, and the third byte specifies the MSB of the `addr`.
The instruction size is 3 bytes.

### ABX (Absolute plus X)
The `addr` is calculated by adding the X register to the address
contained in the second (LSB) and third (MSB) bytes of
the instruction. The instruction size is 3 bytes.

### ABY (Absolute plus Y)
The `addr` is calculated by adding the Y register to the address
contained in the second (LSB) and third (MSB) bytes of
the instruction. The instruction size is 3 bytes.

### REL (Relative)
The `addr` is calculated by adding the second byte of
the instruction to the address of the instruction byte.
The second byte of the instruction in this case is treated as
a signed number from -128 to +127.
The instruction size is 2 bytes.

### ZPG (Zero page)
The second byte of the instruction specifies the LSB of
the `addr`, and the MSB of the `addr` is 0.
The instruction size is 2 bytes.

### ZPX (Zero page plus X)
The LSB of the `addr` is calculated by adding the X register to
the second byte of the instruction, and the MSB of the `addr`
is 0. The instruction size is 2 bytes.

### ZPY (Zero page plus Y)
The LSB of the `addr` is calculated by adding the Y register to
the second byte of the instruction, and the MSB of the `addr`
is 0. The instruction size is 2 bytes.

### VEC (Vector)
The second (LSB) and third (MSB) bytes of the instruction form
an address (a vector). The byte at the formed address is
the LSB of the `addr`, and the next byte is the MSB of
the `addr`. The instruction size is 3 bytes.

### ZVX (Zero page vector plus X)
The second byte of the instruction points to a byte in zero page.
That byte (LSB) and the next byte (MSB) form the proto address.
The MSB of the address of the next byte is always 0, that means
the next byte cannot "escape" zero page. The `addr` is calculated
by adding the X register to the proto address.
The instruction size is 2 bytes.

### ZYV (Zero page plus Y vector)
The second byte of the instruction and the Y register are added.
The result of this addition is a zero page address which points
to a byte. That byte (LSB) and the next byte (MSB) form the `addr`.
The MSB of the address of the next byte is always 0, that means
the next byte cannot "escape" zero page.
The instruction size is 2 bytes.

## Instructions

### INV (Invalid Instruction)
The invalid instruction is used to reserve opcodes.
The behavior of this instruction is implementation defined. 
This instruction should not be used by the program.

### NOP (No operation)
This istruction does nothing.

### BRK (Break Request)
Request a `BREAK` interrupt.

### RTI (Return from an interrupt)
Pull the status flags.
Pull the LSB of the program counter and then
pull the MSB of the program counter.

### RET (Return from a subroutine)
Pull the LSB of the program counter and then
pull the MSB of the program counter.

### FOR (F bitwise OR)
Compute the bitwise OR between the status flags and the byte
at the `addr`. Save the result to the status flags.

### FND (F bitwise AND)
Compute the bitwise AND between the status flags and the byte
at the `addr`. Save the result to the status flags.

### Increment and Decrement
Increment or decrement a register or a memory location by 1.

| Name | Description | Update Flags |
| ---- | ----------- | -------------- |
| `APP` | Increment A | N, Z |
| `AMM` | Decrement A | N, Z |
| `SPP` | Increment S | N, Z |
| `SMM` | Decrement S | N, Z |
| `XPP` | Increment X | N, Z |
| `XMM` | Decrement X | N, Z |
| `YPP` | Increment Y | N, Z |
| `YMM` | Decrement Y | N, Z |
| `INC` | Increment the byte at the `addr` | N, Z |
| `DEC` | Decrement the byte at the `addr` | N, Z |

### Conditional Jumps
Set the program counter to the value of the `addr` if a flag is set.

| Name | Description | Update Flags |
| ---- | ----------- | -------------- |
| `JFB` | Jump if the B flag is 0 |  |
| `JFC` | Jump if the C flag is 0 |  |
| `JFD` | Jump if the D flag is 0 |  |
| `JFN` | Jump if the N flag is 0 |  |
| `JFR` | Jump if the R flag is 0 |  |
| `JFU` | Jump if the U flag is 0 |  |
| `JFV` | Jump if the V flag is 0 |  |
| `JFZ` | Jump if the Z flag is 0 |  |
| `JTB` | Jump if the B flag is 1 |  |
| `JTC` | Jump if the C flag is 1 |  |
| `JTD` | Jump if the D flag is 1 |  |
| `JTN` | Jump if the N flag is 1 |  |
| `JTR` | Jump if the R flag is 1 |  |
| `JTU` | Jump if the U flag is 1 |  |
| `JTV` | Jump if the V flag is 1 |  |
| `JTZ` | Jump if the Z flag is 1 |  |

### JMP
Set the program counter to the value of the `addr`.

### CAL
Push the MSB of the program counter.
Push the LSB of the program counter.
Set the program counter to the value of the `addr`.

### CLC (Clear C)
Set the carry flag to 0.

### Load and Store Operations
Load the byte at the `addr` and store it in a register.
Store the value of a register to the byte at the `addr`.

| Name | Description | Update Flags |
| ---- | ----------- | -------------- |
| `LDA` | Load the accumulaor | N, Z |
| `LDX` | Load the X regiter | N, Z |
| `LDY` | Load the Y regiter | N, Z |
| `STA` | Store the accumulator |  |
| `STX` | Store the X regiter |  |
| `STY` | Store the Y regiter |  |

### Push and Pull Operations
Push or pull a register.

| Name | Description | Update Flags |
| ---- | ----------- | -------------- |
| `PLA` | Pull the accumulaor | N, Z |
| `PLF` | Pull the status flags | All |
| `PLX` | Pull the X register | N, Z |
| `PLY` | Pull the Y register | N, Z |
| `PSA` | Push the accumulator |  |
| `PSA` | Push the status flags |  |
| `PSX` | Push the X register |  |
| `PSY` | Push the Y register |  |

### Data Transfer Operations
Write the value of the accumulator to another register.
Write the value of a register to the accumulator.

| Name | Description | Update Flags |
| ---- | ----------- | -------------- |
| `TAF` | Transfer A to F | All |
| `TAS` | Transfer A to S | N, Z |
| `TAX` | Transfer A to X | N, Z |
| `TAY` | Transfer A to Y | N, Z |
| `TFA` | Transfer F to A | N, Z |
| `TSA` | Transfer S to A | N, Z |
| `TXA` | Transfer X to A | N, Z |
| `TYA` | Transfer Y to A | N, Z |

### CMP
Subtract the byte at the `addr` from the accumulator.
Do NOT save the result. Update the C, N, V, and Z flags.

### CPX
Subtract the byte at the `addr` from the X register.
Do NOT save the result. Update the C, N, V, and Z flags.

### CPY
Subtract the byte at the `addr` from the Y register.
Do NOT save the result. Update the C, N, V, and Z flags.

### SBC & SUB
If it's a SUB instruction set the carry flag to 1.
Subtract the byte at the `addr` from the accumulator, also
subtract 1 if the carry flag is set to 0. The result is stored
in the accumulator. Update the C, N, V, and Z flags.

### ADC & ADD
If it's an ADD instruction set the carry flag to 0.
Add the byte at the `addr` and the accumulator, also
add 1 if the carry flag is set to 1. The result is stored
in the accumulator. Update the C, N, V, and Z flags.

### BOR
Calculate the bitwise OR between the accumulator and
the byte at the `addr`. Update the N and Z flags.
The result is stored in the accumulator.

### XOR
Calculate the bitwise Exclusive OR between the accumulator
and the byte at the `addr`. Update the N and Z flags.
The result is stored in the accumulator.

### AND & BIT
Calculate the bitwise AND between the accumulator and
the byte at the `addr`. Update the N and Z flags.
The result is stored in the accumulator, but if it's
a `BIT` instruction then the result is NOT stored.

### NOT
Calculate the bitwise NOT for the byte at the `addr`.
Update the N and Z flags. Store the result in the byte
at the `addr`.

### NTA
Calculate the bitwise NOT for the accumulator. Update
the N and Z flags. The result is stored in the accumulator.

### SHL
Shift bits of the byte at the `addr` to the left.
The carry flag goes to the 1st bit of the byte.
The 8th bit of the byte goes to the carry flag.
Update the C, N, and Z flags.
```
C <- 87654321 <- C
```

### SHR
Shift bits of the byte at the `addr` to the right.
The carry flag goes to the 8th bit of the byte.
The 1st bit of the byte goes to the carry flag.
Update the C, N, and Z flags.
```
C -> 87654321 -> C
```

### SLA
Shift bits of the accumulator to the left. The carry flag
goes to the 1st bit of the accumulator. The 8th bit of
the accumulator goes to the carry flag. Update the C, N,
and Z flags.
```
C <- 87654321 <- C
```

### SRA
Shift bits of the accumulator to the right. The carry flag
goes to the 8th bit of the accumulator. The 1st bit of
the accumulator goes to the carry flag. Update the C, N,
and Z flags.
```
C -> 87654321 -> C
```

### ZRA, ZRX, ZRY
Set the value of a register to zero.

| Name | Description | Update Flags |
| ---- | ----------- | -------------- |
| `ZRA` | Zero A | N, Z |
| `ZRX` | Zero X | N, Z |
| `ZRY` | Zero Y | N, Z |

## Opcodes

The next 2 tables map addressing modes and instructions to
the values of an instruction byte. Also tells how many cycles
it takes for the XL to finnish an instruction.

Opcode `$00` will always be an invalid instruction.

### From $00 to $7F

| `///` | `$0_` | `$1_` | `$2_` | `$3_` | `$4_` | `$5_` | `$6_` | `$7_` |
| ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- |
| `$_0` | `INV` `NAM` 1 | `JFB` `REL` 2 | `PHA` `NAM` 2 | `LDA` `IMM` 2 | `ZRX` `NAM` 1 | `CMP` `IMM` 2 | `STX` `ABS` 4 | `NTA` `NAM` 1 |
| `$_1` | `BRK` `NAM` 1 | `JFC` `REL` 2 | `PHF` `NAM` 2 | `LDA` `ABS` 4 | `LDX` `IMM` 2 | `CMP` `ABS` 4 | `STX` `ABY` 4 | `CAL` `ABS` 5 |
| `$_2` | `RTI` `NAM` 4 | `JFD` `REL` 2 | `PHX` `NAM` 2 | `LDA` `ZPG` 3 | `LDX` `ABS` 4 | `CMP` `ZPG` 3 | `STX` `ZPG` 3 | `CAL` `ZPG` 4 |
| `$_3` | `RET` `NAM` 3 | `JFN` `REL` 2 | `PHY` `NAM` 2 | `LDA` `VEC` 6 | `LDX` `ABY` 4 | `CMP` `VEC` 6 | `STX` `ZPY` 3 | `CAL` `VEC` 7 |
| `$_4` | `FOR` `IMM` 2 | `JFR` `REL` 2 | `PLA` `NAM` 2 | `LDA` `ABX` 4 | `LDX` `ZPG` 3 | `CMP` `ABX` 4 | `STX` `VEC` 6 | `CAL` `ABX` 5 |
| `$_5` | `FND` `IMM` 2 | `JFU` `REL` 2 | `PLF` `NAM` 2 | `LDA` `ABY` 4 | `LDX` `ZPY` 3 | `CMP` `ABY` 4 | `STX` `ZYV` 5 | `CAL` `ABY` 5 |
| `$_6` | `CLC` `NAM` 1 | `JFV` `REL` 2 | `PLX` `NAM` 2 | `LDA` `ZPX` 3 | `LDX` `VEC` 6 | `CMP` `ZPX` 3 | `LDA` `ZVX` 5 | `CAL` `ZPX` 4 |
| `$_7` | `NOP` `NAM` 1 | `JFZ` `REL` 2 | `PLY` `NAM` 2 | `LDA` `ZPY` 3 | `LDX` `ZYV` 5 | `CMP` `ZPY` 3 | `LDA` `ZYV` 5 | `CAL` `ZPY` 4 |
| `$_8` | `APP` `NAM` 1 | `JTB` `REL` 2 | `TAF` `NAM` 1 | `ZRA` `NAM` 1 | `ZRX` `NAM` 1 | `JMP` `REL` 2 | `STY` `ABS` 4 | `CAL` `ZVX` 6 |
| `$_9` | `AMM` `NAM` 1 | `JTC` `REL` 2 | `TAS` `NAM` 1 | `STA` `ABS` 4 | `LDY` `IMM` 2 | `JMP` `ABS` 3 | `STY` `ABX` 4 | `CAL` `ZYV` 6 |
| `$_A` | `SPP` `NAM` 1 | `JTD` `REL` 2 | `TAX` `NAM` 1 | `STA` `ZPG` 3 | `LDY` `ABS` 4 | `JMP` `ZPG` 2 | `STY` `ZPG` 3 | `JMP` `ZVX` 4 |
| `$_B` | `SMM` `NAM` 1 | `JTN` `REL` 2 | `TAY` `NAM` 1 | `STA` `VEC` 6 | `LDY` `ABX` 4 | `JMP` `VEC` 5 | `STY` `ZPX` 3 | `JMP` `ZYV` 4 |
| `$_C` | `XPP` `NAM` 1 | `JTR` `REL` 2 | `TFA` `NAM` 1 | `STA` `ABX` 4 | `LDY` `ZPG` 3 | `JMP` `ABX` 3 | `STY` `VEC` 6 | `CMP` `ZVX` 5 |
| `$_D` | `XMM` `NAM` 1 | `JTU` `REL` 2 | `TSA` `NAM` 1 | `STA` `ABY` 4 | `LDY` `ZPX` 3 | `JMP` `ABY` 3 | `STY` `ZVX` 5 | `CMP` `ZYV` 5 |
| `$_E` | `YPP` `NAM` 1 | `JTV` `REL` 2 | `TXA` `NAM` 1 | `STA` `ZPX` 3 | `LDY` `VEC` 6 | `JMP` `ZPX` 2 | `STA` `ZVX` 5 | `SLA` `NAM` 1 |
| `$_F` | `YMM` `NAM` 1 | `JTZ` `REL` 2 | `TYA` `NAM` 1 | `STA` `ZPY` 3 | `LDY` `ZVX` 5 | `JMP` `ZPY` 2 | `STA` `ZYV` 5 | `SRA` `NAM` 1 |

### From $80 to $FF

| `///` | `$8_` | `$9_` | `$A_` | `$B_` | `$C_` | `$D_` | `$E_` | `$F_` |
| ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- |
| `$_0` | `INC` `ABS` 5 | `DEC` `ABS` 5 | `BIT` `IMM` 2 | `BOR` `IMM` 2 | `ADC` `IMM` 2 | `ADD` `IMM` 2 | `BIT` `ZVX` 5 | `NOT` `ZPG` 4 |
| `$_1` | `INC` `ABX` 5 | `DEC` `ABX` 5 | `BIT` `ABS` 4 | `BOR` `ABS` 4 | `ADC` `ABS` 4 | `ADD` `ABS` 4 | `BIT` `ZYV` 5 | `NOT` `ZPX` 4 |
| `$_2` | `INC` `ABY` 5 | `DEC` `ABY` 5 | `BIT` `ZPG` 3 | `BOR` `ZPG` 3 | `ADC` `ZPG` 3 | `ADD` `ZPG` 3 | `AND` `ZVX` 5 | `NOT` `ABS` 5 |
| `$_3` | `INC` `ZPG` 4 | `DEC` `ZPG` 4 | `BIT` `VEC` 6 | `BOR` `VEC` 6 | `ADC` `VEC` 6 | `ADD` `VEC` 6 | `AND` `ZYV` 5 | `NOT` `ABX` 5 |
| `$_4` | `INC` `ZPX` 4 | `DEC` `ZPX` 4 | `BIT` `ABX` 4 | `BOR` `ABX` 4 | `ADC` `ABX` 4 | `ADD` `ABX` 4 | `BOR` `ZVX` 5 | `SHL` `ZPG` 4 |
| `$_5` | `INC` `ZPY` 4 | `DEC` `ZPY` 4 | `BIT` `ABY` 4 | `BOR` `ABY` 4 | `ADC` `ABY` 4 | `ADD` `ABY` 4 | `BOR` `ZYV` 5 | `SHL` `ZPX` 4 |
| `$_6` | `INC` `VEC` 7 | `DEC` `VEC` 7 | `BIT` `ZPX` 3 | `BOR` `ZPX` 3 | `ADC` `ZPX` 3 | `ADD` `ZPX` 3 | `XOR` `ZVX` 5 | `SHL` `ABS` 5 |
| `$_7` | `INC` `ZVX` 6 | `DEC` `ZVX` 6 | `BIT` `ZPY` 3 | `BOR` `ZPY` 3 | `ADC` `ZPY` 3 | `ADD` `ZPY` 3 | `XOR` `ZYV` 5 | `SHL` `ABX` 5 |
| `$_8` | `INC` `ZYV` 6 | `DEC` `ZYV` 6 | `AND` `IMM` 2 | `XOR` `IMM` 2 | `SBC` `IMM` 2 | `SUB` `IMM` 2 | `ADC` `ZVX` 5 | `SHR` `ZPG` 4 |
| `$_9` | `CPX` `IMM` 2 | `CPY` `IMM` 2 | `AND` `ABS` 4 | `XOR` `ABS` 4 | `SBC` `ABS` 4 | `SUB` `ABS` 4 | `ADC` `ZYV` 5 | `SHR` `ZPX` 4 |
| `$_A` | `CPX` `ABS` 4 | `CPY` `ABS` 4 | `AND` `ZPG` 3 | `XOR` `ZPG` 3 | `SBC` `ZPG` 3 | `SUB` `ZPG` 3 | `SBC` `ZVX` 5 | `SHR` `ABS` 5 |
| `$_B` | `CPX` `ABY` 4 | `CPY` `ABX` 4 | `AND` `VEC` 6 | `XOR` `VEC` 6 | `SBC` `VEC` 6 | `SUB` `VEC` 6 | `SBC` `ZYV` 5 | `SHR` `ABX` 5 |
| `$_C` | `CPX` `ZPG` 3 | `CPY` `ZPG` 3 | `AND` `ABX` 4 | `XOR` `ABX` 4 | `SBC` `ABX` 4 | `SUB` `ABX` 4 | `ADD` `ZVX` 5 | `INV` `NAM` 1 |
| `$_D` | `CPX` `ZPY` 3 | `CPY` `ZPX` 3 | `AND` `ABY` 4 | `XOR` `ABY` 4 | `SBC` `ABY` 4 | `SUB` `ABY` 4 | `ADD` `ZYV` 5 | `INV` `NAM` 1 |
| `$_E` | `CPX` `VEC` 6 | `CPY` `VEC` 6 | `AND` `ZPX` 3 | `XOR` `ZPX` 3 | `SBC` `ZPX` 3 | `SUB` `ZPX` 3 | `SUB` `ZVX` 5 | `INV` `NAM` 1 |
| `$_F` | `CPX` `ZYV` 5 | `CPY` `ZVX` 5 | `AND` `ZPY` 3 | `XOR` `ZPY` 3 | `SBC` `ZPY` 3 | `SUB` `ZPY` 3 | `SUB` `ZYV` 5 | `INV` `NAM` 1 |

## Examples

## License

This work is licensed under the Creative Commons
Attribution-NonCommercial-ShareAlike 3.0 Unported License.
To view a copy of this license, visit
[https://creativecommons.org/licenses/by-nc-sa/3.0](https://creativecommons.org/licenses/by-nc-sa/3.0) or send
a letter to Creative Commons, PO Box 1866, Mountain View,
CA 94042, USA.
