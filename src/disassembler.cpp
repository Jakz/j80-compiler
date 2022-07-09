#include "disassembler.h"

using namespace Assembler;

std::string op::mnemonic() const
{
  return Opcodes::printInstruction(data).value;
  
  
  /*switch (opcode())
  {
    case OPCODE_NOP: return "NOP";
    case OPCODE_ALU_NNNN: return InstructionALU_NNNN(reg1(), reg2(), alu(), uint16h()).mnemonic();

    default: return "";
  }*/
}


Instruction* J80Disassembler::disassemble(const byte* current)
{
  const op* data = reinterpret_cast<const op*>(current);
  u32 length = data->length();
  
  switch (data->opcode())
  {
    case OPCODE_NOP: return new InstructionNOP();
    case OPCODE_ALU_REG:
    {
      return new InstructionALU_R(data->reg1(), data->reg2(), data->reg3(), data->alu());
    }
    
  }
  
  
  return nullptr;
}
