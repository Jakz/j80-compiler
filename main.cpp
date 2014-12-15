//
//  main.cpp
//  J80 Compiler
//
//  Created by Jack on 4/26/14.
//  Copyright (c) 2014 Jack. All rights reserved.
//

#include <iostream>
#include <cstdio>

#include "assembler.h"

int main(int argc, const char * argv[])
{
  /*Assembler::placeLabel("test1");
  Assembler::assembleSimple(LD_REG_NN, REG_A, (u8)50);
  Assembler::placeLabel("test2");
  Assembler::assembleSimple(LD_REG_PTRNNNN, REG_X, (u16)1234);
  Assembler::assembleJumpImm(COND_ZERO, "test1");
  Assembler::assembleJumpImm(COND_UNCOND, "test2");*/
  
  assemble("test.j80");

  Assembler::assemble();
  
  u16 pos = 0;
  char buffer[64];
  while (pos < Assembler::codeSegment->length)
  {
    printf("%04X: ", pos);
    
    u8 length = Opcodes::printInstruction(Assembler::codeSegment->data+pos, buffer);
    
    for (int w = 0; w < length; ++w)
    {
      printf("%02X", Assembler::codeSegment->data[pos+w]);
    }
    //fprintf(bin,"\n");
    
    if (length == 1) printf("      ");
    if (length == 2) printf("    ");
    if (length == 3) printf("  ");
    printf("  ");
    
    pos += length;

    printf("%s\n", buffer);
  }
  
  u16 dataLen = Assembler::dataSegment->length;
  for (int i = 0; i < dataLen / 8 + (dataLen % 8 != 0 ? 1 : 0); ++i)
  {
    printf("%04X: ", pos + i*8);
    
    for (int j = 0; j < 8; ++j)
    {
      if (i*8+j < dataLen)
        printf("%02X", Assembler::dataSegment->data[i*8 + j]);
      else
        printf("  ");
    }
    
    printf(" ");
    
    for (int j = 0; j < 8; ++j)
    {
      if (i*8+j < dataLen &&  Assembler::dataSegment->data[i*8 + j] >= 0x20 && Assembler::dataSegment->data[i*8 + j] <= 0x7E)
        printf("%c", Assembler::dataSegment->data[i*8 + j]);
      else
        printf(" ");
    }
    
    printf("\n");
  }
  
  FILE *bin = fopen("test.bin", "wb");
  fprintf(bin, "v2.0 raw\n");
  
  for (int i = 0; i < Assembler::codeSegment->length; ++i)
    fprintf(bin, "%02x\n", Assembler::codeSegment->data[i]/*, w < i.length - 1 ? " " : ""*/);
  
  for (int i = 0; i < Assembler::dataSegment->length; ++i)
    fprintf(bin, "%02x\n", Assembler::dataSegment->data[i]);
  
  fclose(bin);
  
  return 0;
}

