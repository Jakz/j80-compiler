#include "instruction.h"

#include "support/ostream.h"

#include "assembler.h"

using namespace Assembler;

#pragma mark InstructionSingleReg
template<typename RegType>
std::string InstructionSingleReg<RegType>::mnemonic() const
{
  return fmt::format("{} {}", Opcodes::opcodeName(opcode), reg);
}

template class Assembler::InstructionSingleReg<Reg8>;
template class Assembler::InstructionSingleReg<Reg16>;

#pragma mark XXX R, S
void InstructionXXX_R_S::assemble(u8* dest) const
{
  dest[0] = (opcode << 3) | reg1;
  dest[1] = (reg2 << 5) | alu;
}

/****************************
 * LD/LSH/RSH R8, S8
 * LD/LSH/RSH R16, R16
 ***************************/
std::string InstructionLD_LSH_RSH::mnemonic() const
{
  const char* srcName = (alu & 0b1) == Alu::EXTENDED_BIT ? Opcodes::reg16(reg1) : Opcodes::reg8(reg1);
  const char* dstName = (alu & 0b1) == Alu::EXTENDED_BIT ? Opcodes::reg16(reg2) : Opcodes::reg8(reg2);
  return fmt::format("{} {}, {}", Opcodes::aluName(alu), srcName, dstName);
}

/****************************
 * CMP R, S
 * CMP P, Q
 ***************************/
std::string InstructionCMP_R_S::mnemonic() const
{
  bool isExtended = (alu & 0b1) == Alu::EXTENDED_BIT;
  
  if (isExtended)
    return fmt::format("{} {}, {}", Opcodes::opcodeName(opcode), Opcodes::reg16(reg1), Opcodes::reg16(reg2));
  else
    return fmt::format("{} {}, {}", Opcodes::opcodeName(opcode), Opcodes::reg8(reg1), Opcodes::reg8(reg2));
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

std::string InstructionXXX_NN::mnemonic() const
{
  return fmt::format("{} {}, {}", Opcodes::opcodeName(opcode), dst, value);
}

/****************************
 * LD R, NN
 ****************************/
void InstructionLD_NN::assemble(u8* dest) const
{
  dest[0] = (opcode << 3) | dst.reg;
  dest[1] = static_cast<byte>(Alu::TRANSFER_B8);
  dest[2] = value.value;
}

/****************************
 * CMP R, NN
 ****************************/
void InstructionCMP_NN::assemble(u8 *dest) const
{
  dest[0] = (opcode << 3) | dst.reg;
  dest[1] = static_cast<byte>(Alu::SUB8);
  dest[2] = value.value;
}

#pragma mark XXX R, NNNN
/****************************
 * XXX R, NNNN
 ****************************/
template<typename RegType>
Result InstructionXXX_NNNN<RegType>::solve(const Environment &env)
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
        env.assembler.log(Log::INFO, true, "  > Data address referenced '%s' (%04X%+d) value", value.label.c_str(), it->second.offset + env.dataSegmentBase, value.offset);
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

template<typename RegType>
std::string InstructionXXX_NNNN<RegType>::mnemonic() const
{
  return fmt::format("{} {}, {}", Opcodes::opcodeName(opcode), dst, value);
}

template class Assembler::InstructionXXX_NNNN<Reg8>;
template class Assembler::InstructionXXX_NNNN<Reg16>;

#pragma mark LD P, NNNN
/****************************
 * LD P, NNNN
 ****************************/
void InstructionLD_NNNN::assemble(u8* dest) const
{
  dest[0] = (OPCODE_LD_NNNN << 3) | dst.reg;
  dest[1] = (value.value >> 8) & 0xFF;
  dest[2] = value.value & 0xFF;
}

#pragma mark ST [NNNN], R
/****************************
 * ST [NNNN], R
 ****************************/
void InstructionST_PTR_NNNN::assemble(u8* dest) const
{
  dest[0] = (OPCODE_SD_PTR_NNNN << 3) | dst.reg;
  dest[1] = (value.value >> 8) & 0xFF;
  dest[2] = value.value & 0xFF;
}

std::string InstructionST_PTR_NNNN::mnemonic() const
{
  return fmt::format("{} [{}], {}", Opcodes::opcodeName(opcode), value, dst);
}

#pragma mark LD R, [NNNN]
void InstructionLD_PTR_NNNN::assemble(u8* dest) const
{
  dest[0] = (OPCODE_LD_PTR_NNNN << 3) | dst.reg;
  dest[1] = (value.value >> 8) & 0xFF;
  dest[2] = value.value & 0xFF;
}

std::string InstructionLD_PTR_NNNN::mnemonic() const
{
  return fmt::format("{} {}, [{}]", Opcodes::opcodeName(opcode), dst, value);
}

#pragma mark CMP P, NNNN
/****************************
 * CMP P, NNNN
 ****************************/
void InstructionCMP_NNNN::assemble(u8 *dest) const
{
  dest[0] = (OPCODE_CMP_NNNN << 3) | dst.reg;
  dest[1] = static_cast<byte>(Alu::SUB16);
  dest[2] = value.value & 0xFF;
  dest[3] = (value.value >> 8) & 0xFF;
}

#pragma mark ALU P, NNNN
/****************************
 * ALU P, NNNN
 ****************************/
std::string InstructionALU_NNNN::mnemonic() const
{
  return fmt::format("{} {}, {}, {}", alu, dst, src, value);
}

void InstructionALU_NNNN::assemble(u8* dest) const
{
  dest[0] = (OPCODE_ALU_NNNN << 3) | dst.reg;
  dest[1] = (src.reg << 5) | alu;
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
  bool extended = (alu & 0b1) == Alu::EXTENDED_BIT;
  
  if (extended)
    return fmt::format("{} {}, {}, {}", alu, Opcodes::reg16(dst), Opcodes::reg16(src1), Opcodes::reg16(src2));
  else
    return fmt::format("{} {}, {}, {}", alu, Opcodes::reg8(dst), Opcodes::reg8(src1), Opcodes::reg8(src2));
}

void InstructionALU_R::assemble(byte *dest) const
{
  dest[0] = (OPCODE_ALU_REG << 3) | dst;
  dest[1] = (src1 << 5) | alu;
  dest[2] = (src2 << 5);
}

#pragma mark ALU R, NN
/****************************
 * ALU R, NN
 ****************************/
std::string InstructionALU_R_NN::mnemonic() const
{
  return fmt::format("{} {}, {}, {}", alu, dst, src, value);
}

void InstructionALU_R_NN::assemble(byte* dest) const
{
  dest[0] = (opcode << 3) | dst.reg;
  dest[1] = (src.reg << 5) | alu;
  dest[2] = value.value;
}

#pragma mark XXX R, [PP]
void InstructionXXX_PTR_PP::assemble(u8* dest) const
{
  dest[0] = (opcode << 3) | dst.reg;
  dest[1] = (raddr.reg << 5) | Alu::ADD_NO_FLAGS;
  dest[2] = value.value;
}

/****************************
 * LD R, [PP+SS]
 * ST [PP+SS], R
 ****************************/
#pragma mark

std::string InstructionLD_PTR_PP::mnemonic() const
{
  if (value.value == 0)
    return fmt::format("{} {}, [{}]", Opcodes::opcodeName(opcode), dst, raddr);
  else
    return fmt::format("{} {}, [{}{:+}]", Opcodes::opcodeName(opcode), dst, raddr, (s8)value.value);
}

std::string InstructionST_PTR_PP::mnemonic() const
{
  if (value.value == 0)
    return fmt::format("{} [{}], {}", Opcodes::opcodeName(opcode), raddr, dst);
  else
    return fmt::format("{} [{}{}], {}", Opcodes::opcodeName(opcode), raddr, (s8)value.value, dst);
}

