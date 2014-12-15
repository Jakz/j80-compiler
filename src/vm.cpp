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

void VM::executeInstruction()
{
  u8 *d = code;
  
  Opcode opcode = static_cast<Opcode>(d[0]>>3);
  Reg reg1 = static_cast<Reg>(d[0] & 0b111);
  Reg reg2 = static_cast<Reg>(d[1] >> 5);
  Reg reg3 = static_cast<Reg>(d[2] >> 5);
  u8 unsigned8 = (u8)d[2];
  s8 signed8 = (s8)d[2];
  u16 short1 = d[1] | (d[2]<<8);
  u16 short2 = d[2] | (d[3]<<8);
  AluOp alu = static_cast<AluOp>(d[1] & 0b11111);
  JumpCondition cond = static_cast<JumpCondition>(d[0] & 0b1111);
  
  switch (opcode)
  {
    // R8 <- R8, R16 <- R16, RSH/LSH R8, RSH/LSH R16
    case OPCODE_LD_RSH_LSH:
    {
      // TODO
    }
      
      // R <- NN
    case OPCODE_LD_NN:
    {
      u8& r = reg8(reg1);
      r = unsigned8;
      break;
    }
     
    // R <- NNNN
    case OPCODE_LD_NNNN:
    {
      u16& r = reg16(reg1);
      r = short1;
      break;
    }
      
    // R <- [NNNN]
    case OPCODE_LD_PTR_NNNN:
    {
      u8& r = reg8(reg1);
      u16 address = short1;
      
      r = ramRead(address);
      break;
    }
      
    // R <- [PP + SS]
    case OPCODE_LD_PTR_PP:
    {
      u8& r = reg8(reg1);
      u16 baseAddress = reg16(reg2);
      s8 offset = signed8;
      
      r = ramRead(baseAddress+offset);
      break;
    }
      
    // [NNNN] <- R
    case OPCODE_SD_PTR_NNNN:
    {
      u8& r = reg8(reg1);
      u16 address = reg16(reg2);
      
      ramWrite(address, r);
      break;
    }
      
    // [PP + SS] <- R
    case OPCODE_SD_PTR_PP:
    {
      u8& r = reg8(reg1);
      u16 baseAddress = reg16(reg2);
      s8 offset = signed8;
      
      ramWrite(baseAddress+offset, r);
      break;
    }
      
    case OPCODE_JMP_NNNN:
    {
      u16 address = short1;
      regs.PC = address;
      break;
    }
      
    case OPCODE_JMP_PP:
    {
      u16 address = reg16(reg2);
      regs.PC = address;
      break;
    }
  }
}