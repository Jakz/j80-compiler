#include "vm.h"

#include "opcodes.h"

bool VM::isConditionTrue(JumpCondition condition) const
{
  u8 flags = regs.FLAGS;
  
  switch (condition) {
    case COND_CARRY: return flags & FLAG_CARRY;
    case COND_NCARRY: return !(flags & FLAG_CARRY);
    case COND_ZERO: return flags & FLAG_ZERO;
    case COND_NZERO: return !(flags & FLAG_ZERO);
    case COND_OVERFLOW: return flags & FLAG_OVERFLOW;
    case COND_NOVERFLOW: return !(flags & FLAG_OVERFLOW);
    case COND_SIGN: return flags & FLAG_SIGN;
    case COND_NSIGN: return !(flags & FLAG_SIGN);
    case COND_UNCOND: return true;
  }
}

void VM::ramWrite(u16 address, u8 value)
{
  if (address == 0xFFFF && sout)
    sout->out(value);
  else
    memory[address] = value;
}

template <typename W> void aluFlagsArithmetic(const W& op1, const W& op2, const W& dest)
{
  setFlag(FLAG_CARRY, op1 + op2 > std::numeric_limits<W>::max());
  setFlag(FLAG_ZERO, dest == 0);
  setFlag(FLAG_SIGN, isNegative(dest));
  setFlag(FLAG_OVERFLOW, !(isNegative(op1) ^ isNegative(op2)) && (isNegative(op1) ^ isNegative(dest)));
}

template <typename W> void VM::alu(Alu op, const W &op1, const W &op2, W &dest, bool saveResult, bool saveFlags)
{
  bool setArithmeticFlags = false;
  s32 result = 0;
  
  switch (op) {
    case Alu::TRANSFER_A8:
    case Alu::TRANSFER_A16:
    {
      dest = op2;
      return;
    }
    
    case Alu::TRANSFER_B8:
    case Alu::TRANSFER_B16:
    {
      dest = op2;
      return;
    }
      
    case Alu::ADD8:
    case Alu::ADD16:
    {
      result = op1 + op2;
      setArithmeticFlags = true;
      setFlag(FLAG_CARRY, result > std::numeric_limits<W>::max());
      break;
    }
    case Alu::ADC8:
    case Alu::ADC16:
    {
      result = op1 + op2 + (isFlagSet(FLAG_CARRY) ? 1 : 0);
      dest = result;
      setFlag(FLAG_CARRY, result > std::numeric_limits<W>::max());
      setArithmeticFlags = true;
      break;
    }
    case Alu::SUB8:
    case Alu::SUB16:
    {
      result = op1 - op2;
      setFlag(FLAG_CARRY, result < 0);
      setArithmeticFlags = true;
      break;
    }
    case Alu::SBC8:
    case Alu::SBC16:
    {
      result = op1 - op2 - (isFlagSet(FLAG_CARRY) ? 1 : 0);
      setFlag(FLAG_CARRY, result < 0);
      setArithmeticFlags = true;
      break;
    }
    case Alu::AND8:
    case Alu::AND16:
      dest = op1 & op2;
      break;
    case Alu::OR8:
    case Alu::OR16:
      dest = op1 | op2;
      break;
    case Alu::XOR8:
    case Alu::XOR16:
      dest = op1 ^ op2;
      break;
    case Alu::NOT8:
    case Alu::NOT16:
      dest = ~op1;
      break;
    
    case Alu::LSH16:
    case Alu::LSH8:
    {
      setFlag(FLAG_CARRY, isNegative(op1));
      dest = op1 << 1;
      break;
    }
      
    case Alu::RSH16:
    case Alu::RSH8:
    {
      setFlag(FLAG_CARRY, op1 & 0x01);
      dest = op1 >> 1;
      break;
    }
  }
  
  if (saveResult)
    dest = result;
  
  if (setArithmeticFlags)
  {
    setFlag(FLAG_SIGN, isNegative(result));
    setFlag(FLAG_OVERFLOW, !(isNegative(op1) ^ isNegative(op2)) && (isNegative(op1) ^ isNegative(result)));
  }

  if (saveFlags)
    setFlag(FLAG_ZERO, (saveResult ? dest : W(result)) == 0);
}

void VM::executeInstruction()
{
  u8 *d = &memory[regs.PC];
  
  u8 length = 0;
  
  Opcode opcode = static_cast<Opcode>(d[0]>>3);
  Reg reg1 = static_cast<Reg>(d[0] & 0b111);
  Reg reg2 = static_cast<Reg>(d[1] >> 5);
  Reg reg3 = static_cast<Reg>(d[2] >> 5);
  u8 unsigned8 = (u8)d[2];
  s8 signed8 = (s8)d[2];
  u16 short1 = d[2] | (d[1]<<8);
  u16 short2 = d[2] | (d[3]<<8);
  Alu aluop = static_cast<Alu>(d[1] & 0b11111);
  JumpCondition cond = static_cast<JumpCondition>(d[0] & 0b1111);
  
  bool saveFlags = true;

  switch (opcode)
  {
    // R8 <- R8, R16 <- R16, RSH/LSH R8, RSH/LSH R16
    case OPCODE_LD_RSH_LSH:
    {
      if ((aluop & 0x1) == Alu::EXTENDED_BIT)
        alu<u16>(aluop, reg16(reg1), reg16(reg2), reg16(reg1), true, saveFlags);
      else
        alu<u8>(aluop, reg8(reg1), reg8(reg2), reg8(reg1), true, saveFlags);
      
      length = 2;
      break;
    }
      
    case OPCODE_CMP_NN:
    {
      alu<u8>(aluop, reg8(reg1), unsigned8, reg8(reg1), false, saveFlags);

      length = 3;
      break;
    }

    case OPCODE_ALU_NN:
    {
      alu<u8>(aluop, reg8(reg2), unsigned8, reg8(reg1), true, saveFlags);
      
      length = 3;
      break;
    }
      
    case OPCODE_CMP_NNNN:
    {
      alu<u16>(aluop, reg16(reg1), short2, reg16(reg1), false, saveFlags);
      length = 4;
      break;
    }

    case OPCODE_ALU_NNNN:
    {
      alu<u16>(aluop, reg16(reg2), short2, reg16(reg1), true, saveFlags);
      
      length = 4;
      break;
    }

    case OPCODE_ALU_REG:
    {
      if ((aluop & 0x1) == Alu::EXTENDED_BIT)
        alu<u16>(aluop, reg16(reg2), reg16(reg3), reg16(reg1), true, saveFlags);
      else
        alu<u8>(aluop, reg8(reg2), reg8(reg3), reg8(reg1), true, saveFlags);

      length = 3;
      break;
    }
      
    case OPCODE_CMP_REG:
    {
      if ((aluop & 0x1) == Alu::EXTENDED_BIT)
        alu<u16>(aluop, reg16(reg1), reg16(reg2), reg16(reg1), false, saveFlags);
      else
        alu<u8>(aluop, reg8(reg1), reg8(reg2), reg8(reg1), false, saveFlags);
      
      length = 2;
      break;
    }
      
    // R <- NN
    case OPCODE_LD_NN:
    {
      u8& r = reg8(reg1);
      r = unsigned8;
      
      length = 3;
      break;
    }
     
    // R <- NNNN
    case OPCODE_LD_NNNN:
    {
      u16& r = reg16(reg1);
      r = short1;
      
      length = 3;
      break;
    }
      
    // R <- [NNNN]
    case OPCODE_LD_PTR_NNNN:
    {
      u8& r = reg8(reg1);
      u16 address = short1;
      
      r = ramRead(address);
      
      length = 3;
      break;
    }
      
    // R <- [PP + SS]
    case OPCODE_LD_PTR_PP:
    {
      u8& r = reg8(reg1);
      u16 baseAddress = reg16(reg2);
      s8 offset = signed8;
      
      r = ramRead(baseAddress+offset);
      
      length = 3;
      break;
    }
      
    // [NNNN] <- R
    case OPCODE_SD_PTR_NNNN:
    {
      u8& r = reg8(reg1);
      u16 address = short1;
      
      ramWrite(address, r);
      
      length = 3;
      break;
    }
      
    // [PP + SS] <- R
    case OPCODE_SD_PTR_PP:
    {
      u8& r = reg8(reg1);
      u16 baseAddress = reg16(reg2);
      s8 offset = signed8;
      
      ramWrite(baseAddress+offset, r);
      
      length = 3;
      break;
    }
      
    case OPCODE_JMPC_NNNN:
    case OPCODE_JMP_NNNN:
    {
      if (isConditionTrue(cond))
      {
        u16 address = short1;
        regs.PC = address;
      }
      else
        length = 3;
      break;
    }
      
    case OPCODE_JMPC_PP:
    case OPCODE_JMP_PP:
    {
      if (isConditionTrue(cond))
      {
        u16 address = reg16(reg2);
        regs.PC = address;
      }
      else
        length = 2;

      break;
    }
      
    case OPCODE_NOP:
    {
      length = 1;
      break;
    }
      
    case OPCODE_PUSH:
    {
      u8& r = reg8(reg1);
      u16& sp = reg16(Reg::SP);
      --sp;
      ramWrite(sp, r);
      
      length = 1;
      break;
    }
      
    case OPCODE_PUSH16:
    {
      u16& r = reg16(reg1);
      u16& sp = reg16(Reg::SP);
      --sp;
      ramWrite(sp, r & 0xFF);
      --sp;
      ramWrite(sp, (r >> 8) & 0xFF);
      
      length = 1;
      break;
    }
      
    case OPCODE_POP:
    {
      u8& r = reg8(reg1);
      u16& sp = reg16(Reg::SP);
      r = ramRead(sp);
      ++sp;
      
      length = 1;
      break;
    }
      
    case OPCODE_POP16:
    {
      u16& r = reg16(reg1);
      u16& sp = reg16(Reg::SP);
      u8 high = ramRead(sp);
      ++sp;
      u8 low = ramRead(sp);
      ++sp;
      r = (high << 8)| low;
      
      length = 1;
      break;
    }
      
    case OPCODE_RET:
    case OPCODE_RETC:
    {
      if (isConditionTrue(cond))
      {
        u16& sp = reg16(Reg::SP);
        u8 high = ramRead(sp);
        ++sp;
        u8 low = ramRead(sp);
        ++sp;
        regs.PC = (high << 8) | low;
      }
      else
        length = 1;
      
      break;
    }
      
    case OPCODE_CALL:
    case OPCODE_CALLC:
    {
      if (isConditionTrue(cond))
      {
        u16 address = regs.PC+3;
        u16& sp = reg16(Reg::SP);
        --sp;
        ramWrite(sp, address & 0xFF);
        --sp;
        ramWrite(sp, (address >> 8) & 0xFF);
        regs.PC = short1;
      }
      else length = 3;
      break;
    }
      
    case OPCODE_LF:
    {
      u8& r = reg8(reg1);
      regs.FLAGS = 0x0F & r;
      
      length = 1;
      break;
    }
      
    case OPCODE_SF:
    {
      u8& r = reg8(reg1);
      r = 0x0F & regs.FLAGS;
      
      length = 1;
      break;
    }
      
    case OPCODE_EI:
    {
      interruptEnabled = true;
      
      length = 1;
      break;
    }
      
    case OPCODE_DI:
    {
      interruptEnabled = false;
      
      length = 1;
      break;
    }
      
    case OPCODE_SEXT:
    {
      u8& r = reg8(reg1);
      u8& h = reg8(static_cast<Reg>(reg1 | 0b100));
      h = r & 0x80 ? 0xFF : 0x00;
      
      length = 1;
      break;
    }
  }
  
  regs.PC += length;
}
