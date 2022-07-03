#pragma once

#include "utils.h"
#include "instruction.h"

namespace Assembler
{
  class J80Disassembler
  {
  private:
    
  public:
    Instruction* disassemble(const byte* current);
  };
}
