#include "vm.h"

#include "opcodes.h"

u8& VM::reg8(Reg r)
{
  switch (r) {
    case REG_A: return regs.A;
    case REG_B: return regs.B;
    case REG_C: return regs.C;
    case REG_D: return regs.D;
    case REG_E: return regs.E;
    case REG_F: return regs.F;
    case REG_X: return regs.X;
    case REG_Y: return regs.Y;
  }
}

u16& VM::reg16(Reg r)
{
  switch (r) {
    case REG_BA: return regs.BA;
    case REG_CD: return regs.CD;
    case REG_EF: return regs.EF;
    case REG_XY: return regs.XY;
    case REG_SP: return regs.SP;
    case REG_FP: return regs.FP;
    case REG_IX: return regs.IX;
    case REG_IY: return regs.IY;
  }
}

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


template <typename W> void VM::add(const W& op1, const W& op2, W& dest, bool flags)
{
  dest = op1 + op2;

  if (flags)
    aluFlagsArithmetic(op1, op2, dest);
}

template <typename W> void VM::adc(const W& op1, const W& op2, W& dest, bool flags)
{
  
}

template <typename W> void VM::sub(const W& op1, const W& op2, W& dest, bool flags)
{
  
}

template <typename W> void VM::sbc(const W& op1, const W& op2, W& dest, bool flags)
{
  
}

template <typename W> void VM::alu(AluOp op, const W &op1, const W &op2, W &dest, bool flags)
{
  bool setArithmeticFlags = false;
  s32 result = 0;
  
  switch (op) {
    case ALU_TRANSFER_A8:
    case ALU_TRANSFER_A16:
    {
      dest = op2;
      return;
    }
    
    case ALU_TRANSFER_B8:
    case ALU_TRANSFER_B16:
    {
      dest = op2;
      return;
    }
      
    case ALU_ADD8:
    case ALU_ADD16:
    {
      result = op1 + op2;
      setArithmeticFlags = true;
      setFlag(FLAG_CARRY, result > std::numeric_limits<W>::max());
      break;
    }
    case ALU_ADC8:
    case ALU_ADC16:
    {
      result = op1 + op2 + (isFlagSet(FLAG_CARRY) ? 1 : 0);
      dest = result;
      setFlag(FLAG_CARRY, result > std::numeric_limits<W>::max());
      setArithmeticFlags = true;
      break;
    }
    case ALU_SUB8:
    case ALU_SUB16:
    {
      result = op1 - op2;
      setFlag(FLAG_CARRY, result < 0);
      setArithmeticFlags = true;
      break;
    }
    case ALU_SBC8:
    case ALU_SBC16:
    {
      result = op1 - op2 - (isFlagSet(FLAG_CARRY) ? 1 : 0);
      setFlag(FLAG_CARRY, result < 0);
      setArithmeticFlags = true;
      break;
    }
    case ALU_AND8:
    case ALU_AND16:
      dest = op1 & op2;
      break;
    case ALU_OR8:
    case ALU_OR16:
      dest = op1 | op2;
      break;
    case ALU_XOR8:
    case ALU_XOR16:
      dest = op1 ^ op2;
      break;
    case ALU_NOT8:
    case ALU_NOT16:
      dest = ~op1;
      break;
    
    case ALU_LSH16:
    case ALU_LSH8:
    {
      setFlag(FLAG_CARRY, isNegative(op1));
      dest = op1 << 1;
      break;
    }
      
    case ALU_RSH16:
    case ALU_RSH8:
    {
      setFlag(FLAG_CARRY, op1 & 0x01);
      dest = op1 >> 1;
      break;
    }
  }
  
  if (flags)
    dest = result;
  
  if (setArithmeticFlags)
  {
    setFlag(FLAG_SIGN, isNegative(result));
    setFlag(FLAG_OVERFLOW, !(isNegative(op1) ^ isNegative(op2)) && (isNegative(op1) ^ isNegative(result)));
  }
  setFlag(FLAG_ZERO, dest == 0);
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
  AluOp aluop = static_cast<AluOp>(d[1] & 0b11111);
  JumpCondition cond = static_cast<JumpCondition>(d[0] & 0b1111);
  
  bool saveFlags = true;
  
  if (opcode == OPCODE_CMP_NN)
  {
    aluop = ALU_SUB8;
    saveFlags = false;
  }
  else if (opcode == OPCODE_CMP_NNNN)
  {
    aluop = ALU_SUB16;
    saveFlags = false;
  }
  else if (opcode == OPCODE_CMP_REG)
  {
    if (aluop & 0x1) aluop = ALU_SUB8;
    aluop = ALU_SUB16;
    saveFlags = false;
  }
  
  
  switch (opcode)
  {
    // R8 <- R8, R16 <- R16, RSH/LSH R8, RSH/LSH R16
    case OPCODE_LD_RSH_LSH:
    {
      if (aluop & 0x1)
        alu<u16>(aluop, reg16(reg1), reg16(reg2), reg16(reg1), saveFlags);
      else
        alu<u8>(aluop, reg8(reg1), reg8(reg2), reg8(reg1), saveFlags);
      
      length = 2;
      break;
    }
      
    case OPCODE_CMP_NN:
    case OPCODE_ALU_NN:
    {
      alu<u8>(aluop, reg8(reg2), unsigned8, reg8(reg1), saveFlags);
      
      length = 3;
      break;
    }
      
    case OPCODE_CMP_NNNN:
    case OPCODE_ALU_NNNN:
    {
      alu<u16>(aluop, reg16(reg2), short2, reg16(reg1), saveFlags);
      
      length = 4;
      break;
    }
      
    case OPCODE_CMP_REG:
    case OPCODE_ALU_REG:
    {
      if (aluop & 0x1)
        alu<u16>(aluop, reg16(reg2), reg16(reg3), reg16(reg1), saveFlags);
      else
        alu<u8>(aluop, reg8(reg2), reg8(reg3), reg8(reg1), saveFlags);
      
      length = 3;
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
      u16& sp = reg16(REG_SP);
      --sp;
      ramWrite(sp, r);
      
      length = 1;
      break;
    }
      
    case OPCODE_PUSH16:
    {
      u16& r = reg16(reg1);
      u16& sp = reg16(REG_SP);
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
      u16& sp = reg16(REG_SP);
      r = ramRead(sp);
      ++sp;
      
      length = 1;
      break;
    }
      
    case OPCODE_POP16:
    {
      u16& r = reg16(reg1);
      u16& sp = reg16(REG_SP);
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
        u16& sp = reg16(REG_SP);
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
        u16& sp = reg16(REG_SP);
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
      u8& h = reg8(static_cast<Reg>(reg1 | 0x100));
      h = r & 0x80 ? 0xFF : 0x00;
      
      length = 1;
      break;
    }
  }
  
  regs.PC += length;
}