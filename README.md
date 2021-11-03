# CS 281: Intro to Computer Systems

## Y86 CPU Project

#### Overview

In this lab, we build our own logisim model of a fully functioning CPU to execute Y86 instructions. You
are given components of the machine to use. You will combine these with the appropriate control logic to
make your CPU function correctly. You will work on your own for this assignment.

#### Hardware Design Components

You are given the following components. You should not modify them in any way. Use them in your CPU
circuit and design the appropriate logic around them.

* y86.circ. This is your main circuit file. Obviously you will modify this file as you will add a substantial
number of other pieces. Please keep this file named as y86.circ.

* imem.circ. This is your instruction memory. It should already be placed on your y86 circuit. There
are four banks of memory in this circuit. Though Y86 is a 64 bit architecture, this memory device is
addressed with the lowest 32 bits of addresses. Look inside the file to see how it works.

* dmem.circ. This is your data memory. It is divided into 2 banks. It is addressed with a 64 bit address
and reads/writes 64 bits of data.

* regs.circ. This is your register file. It is a complex circuit with many inputs and outputs. Be sure to
study the circuit and your textbook to fully understand its operation.


# For more information, read Y86__Copy_.pdf
