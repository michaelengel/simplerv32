#!/bin/sh
set -e

riscv32-unknown-elf-gcc -c -march=rv32i start.S
riscv32-unknown-elf-gcc -c -march=rv32i -o main.o main.c
riscv32-unknown-elf-gcc -Os -T rv32.lds -ffreestanding -nostdlib -o main.elf -Wl,--strip-debug start.o main.o
riscv32-unknown-elf-objcopy -O binary main.elf main.bin

cc -o rv32emu rv32emu.c
./rv32emu main.bin
