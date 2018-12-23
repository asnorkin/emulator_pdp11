# PDP 11 emulator

Legendary PDP 11 architecture emulator written in C++ as a part of Computer Architecture course at MIPT.

Full architecture description: [wiki](https://en.wikipedia.org/wiki/PDP-11_architecture)

## Components
There are number of components implemented in this emulator. Let's see a big picture and then consider each module in more detail.

 - CPU
 - Memory
 - GUI
 - Example program
 
### CPU
Core component of emulator.

CPU processes instructions one by one like real PDP:
 - fetch instruction
 - decode instruction
 - fetch operands
 - execute instruction
 - write back
 
CPU supports 8 operand address modes like real PDP:
 -  REGISTER
 -  REGISTER DEFERRED
 -  AUTOINCREMENT
 -  AUTOINCREMENT DEFERRED
 -  AUTODECREMENT
 -  AUTODECREMENT DEFERRED
 -  INDEX
 -  INDEX DEFERRED
 
Current CPU implementation supports more than a half of real PDP instructions. And other instructions are not implemented just because there wasn't enough time.
 
Also there is instruction cache and writeback buffer emulation that make significantly performance improvement for real PDP.
 
Besides, there is a pipeline like on a any modern CPUs

<img width="50%" height="50%" src="https://upload.wikimedia.org/wikipedia/commons/thumb/c/cb/Pipeline%2C_4_stage.svg/750px-Pipeline%2C_4_stage.svg.png">

The emulator counts how many clocks program will work with and without pipeline and calculates pipeline performance improvement.  
Besides, the pipeline contains variable number of ALUs (2 by default) and OFUs (3 by default).

More about instruction pipelining: [wikipedia](https://en.wikipedia.org/wiki/Instruction_pipelining)


### Memory
The emulator memory module implements RAM, Registers, Flags and VRAM logic.

Memory structure :
 - VRAM: 64016 - 80016 bytes  
 - REGISTERS: 64000 - 64016 bytes  
 - RAM: 0 - 64000 bytes

RAM and VRAM are implemented as sequence of bytes with get, set and reset.

Registers are implemented as set of 8 WORD (2 byte size) registers: R0-R5, SP, PC.

Flagss C, V, Z, N, T are implemented.


### GUI
GUI supports simple IO, debug and visualisation tools like you are using real PDP or even cool :)


### Example program
There is simple program example which only loads image into VRAM and shows it on the screen.
