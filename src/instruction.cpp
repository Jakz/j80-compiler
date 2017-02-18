#include "instruction.h"

#include "assembler.h"

using namespace Assembler;

static const char* m[] =
{
  "nop",
  "ld",//"ld",
  "st",//"st",
  "lsh",
  "rsh",
  "add",
  "adc",
  "sub",
  "sbc",
  "cmp",
  "and",
  "or",
  "xor",
  "not",
  "push",
  "pop",
  "call",
  "jmp",
  "ret",
  "ei",
  "di",
  "lf",
  "sf",
  "sext"
};

enum OpNames
{
  N_NOP,
  N_LD,
  N_ST,
  N_LSH,
  N_RSH,
  N_ADD,
  N_ADC,
  N_SUB,
  N_SBC,
  N_CMP,
  N_AND,
  N_OR,
  N_XOR,
  N_NOT,
  N_PUSH,
  N_POP,
  N_CALL,
  N_JMP,
  N_RET,
  N_EI,
  N_DI,
  N_LF,
  N_SF,
  N_SEXT
};


/****************************
 * LD/LSH/RSH R8, S8
 * LD/LSH/RSH R16, R16
 ***************************/

std::string InstructionLD_LSH_RSH::mnemonic() const
{
  if (alu == ALU_TRANSFER_A8)
    return fmt::sprintf("%s %s, %s", m[N_LD], Opcodes::reg8(src), Opcodes::reg8(dst) );
  else if (alu == ALU_TRANSFER_A16)
    return fmt::sprintf("%s %s, %s", m[N_LD], Opcodes::reg16(src), Opcodes::reg16(dst) );
  else if (alu == ALU_LSH8)
    return fmt::sprintf("%s %s", m[N_LSH], Opcodes::reg8(src));
  else if (alu == ALU_RSH8)
    return fmt::sprintf("%s %s", m[N_RSH], Opcodes::reg8(src));
  else if (alu == ALU_LSH16)
    return fmt::sprintf("%s %s", m[N_LSH], Opcodes::reg16(src));
  else if (alu == ALU_RSH16)
    return fmt::sprintf("%s %s", m[N_RSH], Opcodes::reg16(src));
  else
    assert(false);
}


/****************************
 * LD R, NN
 ****************************/
void InstructionLD_NN::assemble(u8* dest) const
{
  dest[0] = (OPCODE_LD_NN << 3) | dst;
  dest[1] = ALU_TRANSFER_B8;
  dest[2] = value.value;
}

Result InstructionLD_NN::solve(const Environment& env)
{
  switch (value.type)
  {
    case Value8::Type::VALUE: break;
    case Value8::Type::DATA_LENGTH:
    {
      auto it = env.data.map.find(value.label);
      
      if (it == env.data.map.end())
        return Result(fmt::sprintf("reference to missing data '%s'.", value.label.c_str()));
      
      u16 cvalue = it->second.length;
      
      if (!valueFitsType<dest_t>(cvalue))
        return Result(fmt::sprintf("Error: constant %s has a value too large for destination (%u).", value.label.c_str(), cvalue));
      
      env.assembler.log(Log::INFO, true, "  > Data '%s' length: %u", value.label.c_str(), it->second.length);
      value.value = it->second.length;
      break;
    }
    case Value8::Type::CONST:
    {
      auto it = env.consts.find(value.label);
      
      if (it == env.consts.end())
        return Result(fmt::sprintf("reference to missing const '%s'.", value.label.c_str()));
      
      u16 cvalue = it->second;
      
      if (!valueFitsType<dest_t>(cvalue))
        return Result(fmt::sprintf("Error: constant %s has a value too large for destination (%u).", value.label.c_str(), cvalue));
      
      env.assembler.log(Log::INFO, true, "  > Data '%s' value", value.label.c_str());
      value.value = it->second;
      break;
    }
  }
      
  return Result();
}

std::string InstructionLD_NN::mnemonic() const
{
  return fmt::sprintf("%s %s, %.2Xh", m[N_LD], Opcodes::reg8(dst), value.value);
}
