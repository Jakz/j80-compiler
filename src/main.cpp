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

#include "ast.h"

using namespace std;

int main(int argc, const char * argv[])
{
  ASTList* list = new ASTList();
  
  ASTList* list2 = new ASTList();
  list2->add(unique_ptr<ASTNode>(new ASTNumber(50)));
  
  list->add(unique_ptr<ASTNode>(new ASTNumber(10)));
  list->add(unique_ptr<ASTNode>(new ASTUnaryOperator(Unary::NOT, unique_ptr<ASTNode>(new ASTNumber(20)))));
  list->add(unique_ptr<ASTNode>(list2));
  list->recursivePrint(0);
  
  
  
  return 0;
}

/*
int main(int argc, const char * argv[])
{
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
    fprintf(bin, "%02x\n", Assembler::codeSegment->data[i]);
  
  for (int i = 0; i < Assembler::dataSegment->length; ++i)
    fprintf(bin, "%02x\n", Assembler::dataSegment->data[i]);
  
  fclose(bin);
  
  return 0;
}*/
