#include "instruction.h"

#include "support/format/ostream.h"

#include "assembler.h"

#include <cassert>

using namespace Assembler;

//TODO: move them to use them in ui::Screen class

template<>
struct fmt::formatter<Reg16> : fmt::formatter<std::string>
{
  template<typename Context>
  auto format(const Reg16& c, Context& ctx)
  {
    return format_to(ctx.out(), "{}", Opcodes::reg16(c.reg));
  }
};

template<>
struct fmt::formatter<Reg8> : fmt::formatter<std::string>
{
  template<typename Context>
  auto format(const Reg8& c, Context& ctx)
  {
    return format_to(ctx.out(), "{}", Opcodes::reg8(c.reg));
  }
};

template<>
struct fmt::formatter<Alu> : fmt::formatter<std::string>
{
  template<typename Context>
  auto format(const Alu& c, Context& ctx)
  {
    return format_to(ctx.out(), "{}", Opcodes::aluName(c));
  }
};

template<>
struct fmt::formatter<Value8> : fmt::formatter<std::string>
{
  template<typename Context>
  auto format(const Value8& c, Context& ctx)
  {
    return format_to(ctx.out(), "{:02X}h", c.value);
  }
};

template<>
struct fmt::formatter<Value16> : fmt::formatter<std::string>
{
  template<typename Context>
  auto format(const Value16& c, Context& ctx)
  {
    return format_to(ctx.out(), "{:04X}h", c.value);
  }
};

template<>
struct fmt::formatter<Opcode> : fmt::formatter<std::string>
{
  template<typename Context>
  auto format(const Opcode& op, Context& ctx)
  {
    return format_to(ctx.out(), "{}", Opcodes::opcodeName(op));
  }
};

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
  return fmt::format("{} {}, {}", alu, srcName, dstName);
}

/****************************
 * CMP R, S
 * CMP P, Q
 ***************************/
std::string InstructionCMP_R_S::mnemonic() const
{
  bool isExtended = (alu & 0b1) == Alu::EXTENDED_BIT;
  
  if (isExtended)
    return fmt::format("{} {}, {}", opcode, Opcodes::reg16(reg1), Opcodes::reg16(reg2));
  else
    return fmt::format("{} {}, {}", opcode, Opcodes::reg8(reg1), Opcodes::reg8(reg2));
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
        return Result(fmt::format("reference to missing data '{}'.", value.label));
      
      u16 cvalue = it->second.length;
      
      if (!valueFitsType<dest_t>(cvalue))
        return Result(fmt::format("constant {} has a value too large for destination ({}).", value.label, cvalue));
      
      env.assembler.log(Log::INFO, true, "  > Data length referenced '{}' length: {}", value.label, it->second.length);
      value.value = it->second.length;
      break;
    }
    case Value8::Type::CONST:
    {
      auto it = env.consts.find(value.label);
      
      if (it == env.consts.end())
        return Result(fmt::format("reference to missing const '{}'.", value.label));
      
      u16 cvalue = it->second;
      
      if (!valueFitsType<dest_t>(cvalue))
        return Result(fmt::format("constant {} has a value too large for destination ({}).", value.label, cvalue));
      
      env.assembler.log(Log::INFO, true, "  > Data const referenced '{}' value", value.label);
      value.value = it->second;
      break;
    }
  }
  
  return Result();
}

std::string InstructionXXX_NN::mnemonic() const
{
  return fmt::format("{} {}, {}", opcode, dst, value);
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
        return Result(fmt::format("offset specified for length type Value16 '%'", value.label));
      
      auto it = env.data.map.find(value.label);

      if (it == env.data.map.end())
        return Result(fmt::format("reference to missing data '{}'.", value.label));
      
      env.assembler.log(Log::INFO, true, "  > Data length referenced '{}' length: {}", value.label, it->second.length);
      
      value.value = it->second.length;
      break;
    }
    case Type::CONST:
    {
      auto it = env.consts.find(value.label);
      
      if (it == env.consts.end())
        return Result(fmt::format("reference to missing const '{}'.", value.label));

      env.assembler.log(Log::INFO, true, "  > Data const referenced '{}' value", value.label);
      value.value = it->second;
    }
    case Type::LABEL_ADDRESS:
    {
      auto it = env.data.map.find(value.label);
      
      if (it != env.data.map.end())
      {
        env.assembler.log(Log::INFO, true, "  > Data address referenced '{}' ({:04X}{:+d}) value", value.label, it->second.offset + env.dataSegmentBase, value.offset);
        value.value = it->second.offset + env.dataSegmentBase + value.offset;
      }
      else
      {
        auto it = env.consts.find(value.label);
        
        if (it == env.consts.end())
          return Result(fmt::format("reference to missing label '{}'.", value.label));
        
        env.assembler.log(Log::INFO, true, "  > Data const referenced '{}' value", value.label);
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
  return fmt::format("{} {}, {}", opcode, dst, value);
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
  return fmt::format("{} [{}], {}", opcode, value, dst);
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
  return fmt::format("{} {}, [{}]", opcode, dst, value);
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
  assert(alu && Alu::EXTENDED_BIT);
  
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
  assert(!(alu && Alu::EXTENDED_BIT));
  
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


#pragma mark
/****************************
 * LD R, [PP+SS]
 * ST [PP+SS], R
 ****************************/
std::string InstructionLD_PTR_PP::mnemonic() const
{
  if (value.value == 0)
    return fmt::format("{} {}, [{}]", opcode, dst, raddr);
  else
    return fmt::format("{} {}, [{}{:+}]", opcode, dst, raddr, (s8)value.value);
}

std::string InstructionST_PTR_PP::mnemonic() const
{
  if (value.value == 0)
    return fmt::format("{} [{}], {}", opcode, raddr, dst);
  else
    return fmt::format("{} [{}{}], {}", opcode, raddr, (s8)value.value, dst);
}

#pragma mark
/***************************
* JMPC PP
* JMP PP
****************************/
std::string InstructionJMP_PP::mnemonic() const
{
  return fmt::format("{}{} {}", Opcodes::opcodeName(OPCODE_JMPC_PP), Opcodes::condName(condition), reg);
}

#pragma mark
std::string InstructionCALL_NNNN::mnemonic() const
{
  if (address.label.empty())
    return fmt::format("{}{} {:04X}h", Opcodes::opcodeName(OPCODE_CALLC), Opcodes::condName(condition), address.address);
  else
    return fmt::format("{}{} {:04X}h ({})", Opcodes::opcodeName(OPCODE_CALLC), Opcodes::condName(condition), address.address, address.label);
}

std::string InstructionJMP_NNNN::mnemonic() const
{
  if (address.label.empty())
    return fmt::format("{}{} {:04X}h", Opcodes::opcodeName(OPCODE_JMPC_NNNN), Opcodes::condName(condition), address.address);
  else
    return fmt::format("{}{} {:04X}h ({})", Opcodes::opcodeName(OPCODE_JMPC_NNNN), Opcodes::condName(condition), address.address, address.label);
}

#pragma mark
Instruction* Instruction::disassemble(const u8* code)
{
  static constexpr u32 OPCODE_MASK = 0x1F;
  static constexpr u32 OPCODE_SHIFT = 3;
  
  static constexpr u32 ALU_MASK = 0x1F;

  static constexpr u32 REG_MASK = 0x7;

  Opcode opcode = Opcode((code[0] >> OPCODE_SHIFT) & OPCODE_MASK);
  Reg reg1 = Reg(code[0] & REG_MASK);
  Reg reg2 = Reg((code[1] >> 5) & REG_MASK);
  Reg reg3 = Reg((code[2] >> 5) & REG_MASK);
  
  Alu alu = Alu(code[1] & ALU_MASK);
  JumpCondition condition = JumpCondition(code[0] & 0x0F);

  uint8_t uint8 = code[2];

  uint16_t uint16h = code[2] | (code[3] << 8);
  uint16_t uint16l = code[2] | (code[1] << 8);

  bool extended = (u32)alu & (u32)Alu::EXTENDED_BIT;

  switch (opcode)
  {
    case OPCODE_NOP: return new InstructionNOP();
    case OPCODE_SEXT: return new InstructionSEXT(Reg8(reg1));
    case OPCODE_EI: return new InstructionEI();
    case OPCODE_DI: return new InstructionDI();

    case OPCODE_ALU_REG: return new InstructionALU_R(reg1, reg2, reg3, alu);
    case OPCODE_ALU_NN: return new InstructionALU_R_NN(reg1, reg2, alu, uint8);
    case OPCODE_ALU_NNNN: return new InstructionALU_NNNN(reg1, reg2, alu, uint16h);
    case OPCODE_LF: return new InstructionLF(reg1);

    case OPCODE_PUSH16: return new InstructionPUSH16(reg1);
    case OPCODE_SF: return new InstructionSF(reg1);
    case OPCODE_POP16: return new InstructionPOP16(reg1);
    case OPCODE_CMP_REG: return new InstructionCMP_R_S(reg1, reg2, extended);

    case OPCODE_CMP_NN: return new InstructionCMP_NN(reg1, uint8);
    case OPCODE_CMP_NNNN: return new InstructionCMP_NNNN(reg1, uint16h);
    case OPCODE_POP: return new InstructionPOP8(reg1);
    case OPCODE_LD_RSH_LSH: return new InstructionLD_LSH_RSH(reg1, reg2, alu, extended);

    case OPCODE_LD_NN: return new InstructionLD_NN(reg1, uint8);
    case OPCODE_LD_NNNN: return new InstructionLD_NNNN(reg1, uint16l);
    case OPCODE_PUSH: return new InstructionPUSH8(reg1);
    case OPCODE_LD_PTR_NNNN: return new InstructionLD_PTR_NNNN(reg1, uint16l);

    case OPCODE_LD_PTR_PP: return new InstructionLD_PTR_PP(reg1, reg2, uint8);
    case OPCODE_SD_PTR_NNNN: return new InstructionST_PTR_NNNN(reg1, uint16l);
    case OPCODE_SD_PTR_PP: return new InstructionST_PTR_PP(reg1, reg2, uint8);
    case OPCODE_JMPC_NNNN: return new InstructionJMP_NNNN(condition, uint16l);

    case OPCODE_JMP_NNNN: return new InstructionJMP_NNNN(condition, uint16l);
    case OPCODE_JMPC_PP: return new InstructionJMP_PP(condition, reg2);
    case OPCODE_JMP_PP: return new InstructionJMP_PP(condition, reg2);
    case OPCODE_RETC: return new InstructionRET(condition);

    case OPCODE_RET: return new InstructionRET(condition);
    case OPCODE_CALLC: return new InstructionCALL_NNNN(condition, uint16l);
    case OPCODE_CALL: return new InstructionCALL_NNNN(condition, uint16l);
  }
}