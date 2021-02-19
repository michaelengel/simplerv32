//
//
// File Name : riscv.c
// Used on :
// Author : Ted Fried, MicroCore Labs
// Creation : 3/29/2020
// Code Type : C Source
//
// Description:
// ============
//  
// Simple and compact RISC-V RS32I implementation written in C.
//
//------------------------------------------------------------------------
//
// Modification History:
// =====================
//
// Revision 1 3/29/2020
// Initial revision
//
// Revision 2 4/3/2020
// Fixed BLTU
//
//
//------------------------------------------------------------------------
//
// Copyright (c) 2020 Ted Fried
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define U_immediate rv5_opcode >> 12
#define J_immediate_SE (rv5_opcode&0x80000000) ? 0xFFE00000 | (rv5_opcode&0x000FF000) | (rv5_opcode&0x00100000)>>9 | (rv5_opcode&0x80000000)>>11 | (rv5_opcode&0x7FE00000)>>20 : (rv5_opcode&0x000FF000) | (rv5_opcode&0x00100000)>>9 | (rv5_opcode&0x80000000)>>11 | (rv5_opcode&0x7FE00000)>>20  
#define B_immediate_SE (rv5_opcode&0x80000000) ? 0xFFFFE000 | (rv5_opcode&0xF00)>>7 | (rv5_opcode&0x7E000000)>>20 | (rv5_opcode&0x80)<<4 | (rv5_opcode&0x80000000)>> 19 : (rv5_opcode&0xF00)>>7 | (rv5_opcode&0x7E000000)>>20 | (rv5_opcode&0x80)<<4 | (rv5_opcode&0x80000000)>> 19
#define I_immediate_SE (rv5_opcode&0x80000000) ? 0xFFFFF000 | rv5_opcode >> 20 : rv5_opcode >> 20
#define S_immediate_SE (rv5_opcode&0x80000000) ? 0xFFFFF000 | (rv5_opcode&0xFE000000)>>20 | (rv5_opcode&0xF80)>>7 : (rv5_opcode&0xFE000000)>>20 | (rv5_opcode&0xF80)>>7

#define funct7 ((unsigned char) ((rv5_opcode&0xFE000000) >> 25) )
#define rs2 ((unsigned char) ((rv5_opcode&0x01F00000) >> 20) )
#define rs1 ((unsigned char) ((rv5_opcode&0x000F8000) >> 15) )
#define funct3 ((unsigned char) ((rv5_opcode&0x00007000) >> 12) )
#define rd ((unsigned char) ((rv5_opcode&0x00000F80) >> 7 ) )
#define opcode ((rv5_opcode&0x0000007F) )

// uncomment to enable debug printfs
// #define debug(...) fprintf(stderr, __VA_ARGS__)
#define debug(...)

unsigned int shamt;
uint32_t rv5_reg[32];
uint32_t rv5_RAM[16384];
uint32_t rv5_opcode;
uint32_t rv5_pc=0;
uint32_t temp;


int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s file.bin\n", argv[0]);
    exit(0);
  }

  // load binary
  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    printf("Cannot open %s\n", argv[1]);
    exit(0);
  }

  read(fd, rv5_RAM, 65536);

  // go!
  while (1) {

    rv5_opcode = rv5_RAM[rv5_pc>>2]; 
    shamt=rs2;
     
    debug("PC:0x%08x Opcode:0x%08x \n\n", rv5_pc, rv5_opcode);
     
    if (opcode==0b0110111) { rv5_reg[rd] = U_immediate << 12; debug(" LUI "); } else // LUI
    if (opcode==0b0010111) { rv5_reg[rd] = (U_immediate << 12) + rv5_pc; debug(" AUIPC "); } else // AUIPC
    if (opcode==0b1101111) { rv5_reg[rd] = rv5_pc + 0x4; rv5_pc = (J_immediate_SE) + rv5_pc - 0x4; debug(" JAL "); } else // JAL
    if (opcode==0b1100111) { rv5_reg[rd] = rv5_pc + 0x4; rv5_pc = (((I_immediate_SE) + rv5_reg[rs1]) & 0xFFFFFFFE) - 0x4; debug(" JALR "); } else // JALR
    if (opcode==0b1100011 && funct3==0b000) { if (rv5_reg[rs1]==rv5_reg[rs2]) rv5_pc = ( (B_immediate_SE) + rv5_pc) - 0x4; debug(" BEQ "); } else // BEQ
    if (opcode==0b1100011 && funct3==0b001) { if (rv5_reg[rs1]!=rv5_reg[rs2]) rv5_pc = ( (B_immediate_SE) + rv5_pc) - 0x4; debug(" BNE "); } else // BNE
    if (opcode==0b1100011 && funct3==0b100) { if ((signed long)rv5_reg[rs1]< (signed long)rv5_reg[rs2]) rv5_pc = ((B_immediate_SE) + rv5_pc) - 0x4; debug(" BLT "); } else // BLT
    if (opcode==0b1100011 && funct3==0b101) { if ((signed long)rv5_reg[rs1]>=(signed long)rv5_reg[rs2]) rv5_pc = ((B_immediate_SE) + rv5_pc) - 0x4; debug(" BGE "); } else // BGE
    if (opcode==0b1100011 && funct3==0b110) { if (rv5_reg[rs1]<rv5_reg[rs2]) rv5_pc = ( (B_immediate_SE) + rv5_pc) - 0x4; debug(" BLTU ");  } else // BLTU
    if (opcode==0b1100011 && funct3==0b111) { if (rv5_reg[rs1]>=rv5_reg[rs2]) rv5_pc = ( (B_immediate_SE) + rv5_pc) - 0x4; debug(" BGTU "); } else // BGTU
    if (opcode==0b0000011 && funct3==0b000) { rv5_reg[rd] = (rv5_RAM[(I_immediate_SE)+rv5_reg[rs1]] & 0x80) ? 0xFFFFFF00| rv5_RAM[(I_immediate_SE)+rv5_reg[rs1]] : (rv5_RAM[(I_immediate_SE)+rv5_reg[rs1]] & 0xFF); debug(" LB "); } else // LB
    if (opcode==0b0000011 && funct3==0b001) { rv5_reg[rd] = (rv5_RAM[(I_immediate_SE)+rv5_reg[rs1]] & 0x8000) ? 0xFFFF0000| rv5_RAM[(I_immediate_SE)+rv5_reg[rs1]] : (rv5_RAM[(I_immediate_SE)+rv5_reg[rs1]] & 0xFFFF); debug(" LH "); } else // LH
    if (opcode==0b0000011 && funct3==0b010) { rv5_reg[rd] = rv5_RAM[((I_immediate_SE)+rv5_reg[rs1])>>2]; debug(" LW "); } else // LW
    if (opcode==0b0000011 && funct3==0b100) { rv5_reg[rd] = rv5_RAM[((I_immediate_SE)+rv5_reg[rs1])>>2] & 0x000000FF; debug(" LBU "); } else // LBU
    if (opcode==0b0000011 && funct3==0b101) { rv5_reg[rd] = rv5_RAM[((I_immediate_SE)+rv5_reg[rs1])>>2] & 0x0000FFFF; debug(" LHU "); } else // LHU
    if (opcode==0b0100011 && funct3==0b000) { rv5_RAM[(S_immediate_SE)+rv5_reg[rs1]] = (rv5_RAM[((S_immediate_SE)+rv5_reg[rs1])>>2]&0xFFFFFF00) | (rv5_reg[rs2]&0xFF); debug(" SB "); } else // SB
    if (opcode==0b0100011 && funct3==0b001) { rv5_RAM[(S_immediate_SE)+rv5_reg[rs1]] = (rv5_RAM[((S_immediate_SE)+rv5_reg[rs1])>>2]&0xFFFF0000) | (rv5_reg[rs2]&0xFFFF); debug(" SH "); } else // SH
    if (opcode==0b0100011 && funct3==0b010) { 
      uint32_t addr = ((S_immediate_SE)+rv5_reg[rs1]);
      if (addr < 0x10000) { // RAM
        rv5_RAM[((S_immediate_SE)+rv5_reg[rs1])>>2] = rv5_reg[rs2]; debug(" SW "); 
      } else { // IO
        if (addr == 0x80000000) fprintf(stderr, "LEDs: %08x\n", rv5_reg[rs2]);
      }
    } else // SW
    if (opcode==0b0010011 && funct3==0b000) { rv5_reg[rd] = (I_immediate_SE) + rv5_reg[rs1]; debug(" ADDI "); } else // ADDI
    if (opcode==0b0010011 && funct3==0b010) { if ((signed long)rv5_reg[rs1] < ((signed long)I_immediate_SE)) rv5_reg[rd]=1; else rv5_reg[rd]=0; debug(" SLTI "); } else // SLTI
    if (opcode==0b0010011 && funct3==0b011) { if (rv5_reg[rs1] < (I_immediate_SE)) rv5_reg[rd]=1; else rv5_reg[rd]=0; debug(" SLTIU "); } else // SLTIU
    if (opcode==0b0010011 && funct3==0b100) { rv5_reg[rd] = rv5_reg[rs1] ^ (I_immediate_SE); debug(" XORI "); } else // XORI
    if (opcode==0b0010011 && funct3==0b110) { rv5_reg[rd] = rv5_reg[rs1] | (I_immediate_SE); debug(" ORI "); } else // ORI
    if (opcode==0b0010011 && funct3==0b111) { rv5_reg[rd] = rv5_reg[rs1] & (I_immediate_SE); debug(" ANDI "); } else // ANDI
    if (opcode==0b0010011 && funct3==0b001 && funct7==0b0000000) { rv5_reg[rd] = rv5_reg[rs1] << shamt; debug(" SLLI "); } else // SLLI
    if (opcode==0b0010011 && funct3==0b101 && funct7==0b0100000) {rv5_reg[rd]=rv5_reg[rs1]; temp=rv5_reg[rs1]&0x80000000; while (shamt>0) { rv5_reg[rd]=(rv5_reg[rd]>>1)|temp; shamt--;} debug(" SRAI "); } else // SRAI
    if (opcode==0b0010011 && funct3==0b101 && funct7==0b0000000) { rv5_reg[rd] = rv5_reg[rs1] >> shamt; debug(" SRLI "); } else // SRLI
    if (opcode==0b0110011 && funct3==0b000 && funct7==0b0100000) { rv5_reg[rd] = rv5_reg[rs1] - rv5_reg[rs2]; debug(" SUB "); } else // SUB
    if (opcode==0b0110011 && funct3==0b000) { rv5_reg[rd] = rv5_reg[rs1] + rv5_reg[rs2]; debug(" ADD "); } else // ADD
    if (opcode==0b0110011 && funct3==0b001) { rv5_reg[rd] = rv5_reg[rs1] << (rv5_reg[rs2]&0x1F); debug(" SLL "); } else // SLL
    if (opcode==0b0110011 && funct3==0b010) { if ((signed long)rv5_reg[rs1] < (signed long)rv5_reg[rs2]) rv5_reg[rd]=1; else rv5_reg[rd]=0; debug(" SLT "); } else // SLT
    if (opcode==0b0110011 && funct3==0b011) { if (rv5_reg[rs1] < rv5_reg[rs2]) rv5_reg[rd]=1; else rv5_reg[rd]=0; debug(" SLTU "); } else // SLTU
    if (opcode==0b0110011 && funct3==0b100) { rv5_reg[rd] = rv5_reg[rs1] ^ rv5_reg[rs2]; debug(" XOR "); } else // XOR
    if (opcode==0b0110011 && funct3==0b101 && funct7==0b0100000) {rv5_reg[rd]=rv5_reg[rs1]; shamt=(rv5_reg[rs2]&0x1F); temp=rv5_reg[rs1]&0x80000000; while (shamt>0) { rv5_reg[rd]=(rv5_reg[rd]>>1)|temp; shamt--;} debug(" SRA "); } else // SRA
    if (opcode==0b0110011 && funct3==0b101 && funct7==0b0000000) { rv5_reg[rd] = rv5_reg[rs1] >> (rv5_reg[rs2]&0x1F); debug(" SRL "); } else // SRL
    if (opcode==0b0110011 && funct3==0b110) { rv5_reg[rd] = rv5_reg[rs1] | rv5_reg[rs2]; debug(" OR "); } else // OR
    if (opcode==0b0110011 && funct3==0b111) { rv5_reg[rd] = rv5_reg[rs1] & rv5_reg[rs2]; debug(" AND "); } else debug(" **INVALID** "); // AND
   
    rv5_pc = rv5_pc + 0x4;
    rv5_reg[0]=0;
     
    debug("rd:%d rs1:%d rs2:%d U_immediate:0x%x J_immediate:0x%x B_immediate:0x%x I_immediate:0x%x S_immediate:0x%x funct3:0x%x funct7:0x%x\n",rd,rs1,rs2,U_immediate,J_immediate_SE,B_immediate_SE,I_immediate_SE,S_immediate_SE,funct3,funct7);
    debug("Regs: "); for (int i=0; i<32; i++) { debug("r%d:%x ",i,rv5_reg[i]); } debug("\n"); //scanf("%c",&temp);
    // debug("Memory: "); for (int i=0; i<7; i++) { debug("Addr%d:%x ",i,rv5_RAM[i]); } debug("\n"); scanf("%c",&temp);
      
  }
}
