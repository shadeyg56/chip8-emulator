# chip8-emulator
A simple CHIP-8 Emulator in pure C + SDL2

## What is CHIP-8?

CHIP-8 is an interpreted programming language that can only run on a CHIP8 virtual machine. In theory, this is more of an interpreter than an emulator, but they always get called emulators.

## CHIP-8 Architecture

- Memory: 4 KB of RAM
- Display: 64 x 32 monochrome (the emulator scales up to 8x)
- 16 general purpose 8-bit register VF - V0
- 1 index register "I" to point to memory addresses
- A stack of up to 16 16-bit values
- Delay timer
- Sound timer
- Psuedo register program counter that points at current instruction in memory
- Psuedo register stack pointer that points at current location in the stack

## Important Info
This emulator can run any `.ch8` ROMS. 

Please note that due to discrepancies in interpreters some ROMS may not run as intended. 

## Build

Code can be compiled using

```gcc main.c chip-8.c screen.c "-ISDL2\include" "-Iheaders" "-LSDL2\lib" -lmingw32 -lSDL2main -lSDL2 -o main.exe```

## Run
Run the program using

```main.exe /path/to/rom```

## TODO
- Audio

## Acknowledements

I used [this guide](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#8xy6-and-8xye-shift) for all of the info on the architexture and CPU instructions