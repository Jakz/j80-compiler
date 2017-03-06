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
 * XXX R, NN
 ****************************/
Result InstructionXXX_NN::solve(const Environment& env)
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
        return Result(fmt::sprintf("constant %s has a value too large for destination (%u).", value.label.c_str(), cvalue));
      
      env.assembler.log(Log::INFO, true, "  > Data length referenced '%s' length: %u", value.label.c_str(), it->second.length);
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
        return Result(fmt::sprintf("constant %s has a value too large for destination (%u).", value.label.c_str(), cvalue));
      
      env.assembler.log(Log::INFO, true, "  > Data const referenced '%s' value", value.label.c_str());
      value.value = it->second;
      break;
    }
  }
  
  return Result();
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

std::string InstructionLD_NN::mnemonic() const
{
  return fmt::sprintf("%s %s, %.2Xh", m[N_LD], Opcodes::reg8(dst), value.value);
}

/****************************
 * CMP R, NN
 ****************************/
void InstructionCMP_NN::assemble(u8 *dest) const
{
  dest[0] = (OPCODE_CMP_NN << 3) | dst;
  dest[1] = ALU_SUB8;
  dest[2] = value.value;
}

std::string InstructionCMP_NN::mnemonic() const
{
  return fmt::sprintf("%s %s, %.2Xh", m[N_CMP], Opcodes::reg8(dst), value.value);
}

#pragma mark XXX R, NNNN
/****************************
 * XXX R, NNNN
 ****************************/
Result InstructionXXX_NNNN::solve(const Environment &env)
{
  using Type = Value16::Type;
  switch (value.type)
  {
    case Type::VALUE: break;
    case Type::DATA_LENGTH:
    {
      if (value.offset)
        return Result(fmt::sprintf("offset specified for length type Value16 '%'", value.label));
      
      auto it = env.data.map.find(value.label);

      if (it == env.data.map.end())
        return Result(fmt::sprintf("reference to missing data '%s'.", value.label.c_str()));
      
      env.assembler.log(Log::INFO, true, "  > Data length referenced '%s' length: %u", value.label.c_str(), it->second.length);
      
      value.value = it->second.length;
      break;
    }
    case Type::CONST:
    {
      auto it = env.consts.find(value.label);
      
      if (it == env.consts.end())
        return Result(fmt::sprintf("reference to missing const '%s'.", value.label.c_str()));

      env.assembler.log(Log::INFO, true, "  > Data const referenced '%s' value", value.label.c_str());
      value.value = it->second;
    }
    case Type::LABEL_ADDRESS:
    {
      auto it = env.data.map.find(value.label);
      
      if (it != env.data.map.end())
      {
        env.assembler.log(Log::INFO, true, "  > Data address referenced '%s' (%04X+%d) value", value.label.c_str(), it->second.offset + env.dataSegmentBase, value.offset);
        value.value = it->second.offset + env.dataSegmentBase + value.offset;
      }
      else
      {
        auto it = env.consts.find(value.label);
        
        if (it == env.consts.end())
          return Result(fmt::sprintf("reference to missing label '%s'.", value.label.c_str()));
        
        env.assembler.log(Log::INFO, true, "  > Data const referenced '%s' value", value.label.c_str());
        value.value = it->second + value.offset;
      }

      break;
    }
  }
  
  return Result();
}


#pragma mark LD P, NNNN
/****************************
 * LD P, NNNN
 ****************************/
std::string InstructionLD_NNNN::mnemonic() const
{
  if (value.label.empty())
    return fmt::sprintf("%s %s, %.4Xh", m[N_LD], Opcodes::reg16(dst), value.value);
  else
    return fmt::sprintf("%s %s, %.4Xh (%s)", m[N_LD], Opcodes::reg16(dst), value.value, value.label.c_str());
}

void InstructionLD_NNNN::assemble(u8* dest) const
{
  dest[0] = (OPCODE_LD_NNNN << 3) | dst;
  dest[1] = (value.value >> 8) & 0xFF;
  dest[2] = value.value & 0xFF;
}

#pragma mark ST [NNNN], R
/****************************
 * ST [NNNN], R
 ****************************/
std::string InstructionST_NNNN::mnemonic() const
{
  if (value.label.empty())
    return fmt::sprintf("%s %s, %.4Xh", m[N_ST], Opcodes::reg8(dst), value.value);
  else
    return fmt::sprintf("%s %s, %.4Xh (%s)", m[N_ST], Opcodes::reg8(dst), value.value, value.label.c_str());
}

void InstructionST_NNNN::assemble(u8* dest) const
{
  dest[0] = (OPCODE_SD_PTR_NNNN << 3) | dst;
  dest[1] = (value.value >> 8) & 0xFF;
  dest[2] = value.value & 0xFF;
}

#pragma mark CMP P, NNNN
/****************************
 * CMP P, NNNN
 ****************************/
std::string InstructionCMP_NNNN::mnemonic() const
{
  if (value.label.empty())
    return fmt::sprintf("%s %s, %.4Xh", m[N_CMP], Opcodes::reg16(dst), value.value);
  else
    return fmt::sprintf("%s %s, %.4Xh (%s)", m[N_CMP], Opcodes::reg16(dst), value.value, value.label.c_str());
}

void InstructionCMP_NNNN::assemble(u8 *dest) const
{
  dest[0] = (OPCODE_CMP_NNNN << 3) | dst;
  dest[1] = ALU_SUB16;
  dest[2] = value.value & 0xFF;
  dest[3] = (value.value >> 8) & 0xFF;
}

#pragma mark ALU R, S, U
/****************************
 * ALU R, S, U
 * ALU P, Q, O
 ****************************/
std::string InstructionALU_R::mnemonic() const
{
  bool extended = (alu & 0b1) != 0;
  
  if (extended)
    return fmt::sprintf("%s %s, %s, %s", Opcodes::aluName((AluOp)(alu & 0b11110)), Opcodes::reg16(dst), Opcodes::reg16(src1), Opcodes::reg16(src2));
  else
    return fmt::sprintf("%s %s, %s, %s", Opcodes::aluName(alu), Opcodes::reg8(dst), Opcodes::reg8(src1), Opcodes::reg8(src2));
}

void InstructionALU_R::assemble(byte *dest) const
{
  dest[0] = (OPCODE_ALU_REG << 3) | dst;
  dest[1] = (src1 << 5) | alu;
  dest[2] = (src2 << 5);
}
