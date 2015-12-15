#include "opcodes.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "assembler.h"

const char* Opcodes::reg8(u8 reg)
{
  switch (reg)
  {
    case REG_A: return "A";
    case REG_B: return "B";
    case REG_X: return "X";
    case REG_Y: return "Y";
    case REG_C: return "C";
    case REG_D: return "D";
    case REG_E: return "E";
    case REG_F: return "F";
    default: return "ERR";
  }
}

const char* Opcodes::reg16(u8 reg)
{
  switch (reg)
  {
    case REG_BA: return "BA";
    case REG_XY: return "XY";
    case REG_CD: return "CD";
    case REG_EF: return "EF";
    case REG_SP: return "SP";
    case REG_FP: return "FP";
    case REG_IX: return "IX";
    case REG_IY: return "IY";
    default: return "ERR";
  }
}

const char* Opcodes::condName(JumpCondition cond)
{
  switch (cond)
  {
    case COND_UNCOND: return "";
    case COND_CARRY: return "C";
    case COND_NCARRY: return "NC";
    case COND_ZERO: return "Z";
    case COND_NZERO: return "NZ";
    case COND_OVERFLOW: return "V";
    case COND_NOVERFLOW: return "NV";
    case COND_SIGN: return "N";
    case COND_NSIGN: return "NN";
    default: { printf("ERRORE COND NAME! %d\n", cond); return "ERR!"; }
  }
}

/*void Opcodes::printInstruction(Instruction& instruction)
{
  printInstruction(instruction.data);
}*/

const char* m[] =
{
  "nop",
  "mov",//"ld",
  "mov",//"st",
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

const char* Opcodes::aluName(AluOp alu)
{
  switch (alu)
  {
    case ALU_ADD8: return m[N_ADD];
    case ALU_ADC8: return m[N_ADC];
    case ALU_SUB8: return m[N_SUB];
    case ALU_SBC8: return m[N_SBC];
    case ALU_AND8: return m[N_AND];
    case ALU_OR8: return m[N_OR];
    case ALU_XOR8: return m[N_XOR];
    case ALU_NOT8: return m[N_NOT];
    default: { printf("ERROR ALU NAME! %d\n", alu); return "ERR"; }
  }
}

MnemonicInfo Opcodes::printInstruction(const u8 *data)
{
  const u8 *d = data;
  
  Opcode opcode = static_cast<Opcode>(d[0]>>3);
  Reg reg1 = static_cast<Reg>(d[0] & 0b111);
  Reg reg2 = static_cast<Reg>(d[1] >> 5);
  Reg reg3 = static_cast<Reg>(d[2] >> 5);
  u8 unsigned8 = (u8)d[2];
  s8 signed8 = (s8)d[2];
  u16 short1 = d[2] | (d[1]<<8);
  u16 short2 = d[2] | (d[3]<<8);
  AluOp alu = static_cast<AluOp>(d[1] & 0b11111);
  JumpCondition cond = static_cast<JumpCondition>(d[0] & 0b1111);

  MnemonicInfo info = {"N/A", 0};
  
  switch (opcode) {
    case OPCODE_LD_RSH_LSH:
    {
      if (alu == ALU_TRANSFER_A8)
        info.value = fmt::sprintf("%s %s, %s", m[N_LD], reg8(reg1), reg8(reg2) );
      else if (alu == ALU_TRANSFER_A16)
        info.value = fmt::sprintf("%s %s, %s", m[N_LD], reg16(reg1), reg16(reg2) );
      else if (alu == ALU_LSH8)
        info.value = fmt::sprintf("%s %s", m[N_LSH], reg8(reg1));
      else if (alu == ALU_RSH8)
        info.value = fmt::sprintf("%s %s", m[N_RSH], reg8(reg1));
      else if (alu == ALU_LSH16)
        info.value = fmt::sprintf("%s %s", m[N_LSH], reg16(reg1));
      else if (alu == ALU_RSH16)
        info.value = fmt::sprintf("%s %s", m[N_RSH], reg16(reg1));

      info.length = 2;
      break;
    }
      
    case OPCODE_LD_NN: { info = {fmt::sprintf("%s %s, %.2Xh", m[N_LD], reg8(reg1), unsigned8), 3}; break; }
    case OPCODE_LD_NNNN: { info = {fmt::sprintf("%s %s, %.4Xh", m[N_LD], reg16(reg1), short1), 3}; break; }
    case OPCODE_LD_PTR_NNNN: { info = {fmt::sprintf("%s %s, [%.4Xh]", m[N_LD], reg8(reg1), short1), 3}; break; }
    case OPCODE_LD_PTR_PP: { info = {fmt::sprintf("%s %s, [%s%+d]", m[N_LD], reg8(reg1), reg16(reg2), signed8), 3}; break; }
      
    case OPCODE_SD_PTR_NNNN: { info = {fmt::sprintf("%s [%.4Xh], %s ", m[N_ST], short1, reg8(reg1)), 3}; break; }
    case OPCODE_SD_PTR_PP: { info = {fmt::sprintf("%s [%s%+d], %s ", m[N_ST], reg16(reg2), signed8, reg8(reg1)), 3}; break; }

    case OPCODE_ALU_REG:
    {
      bool extended = (alu & 0b1) != 0;
      
      if (extended)
        info.value = fmt::sprintf("%s %s, %s, %s", aluName((AluOp)(alu & 0b11110)), reg16(reg1), reg16(reg2), reg16(reg3));
      else
        info.value = fmt::sprintf("%s %s, %s, %s", aluName(alu), reg8(reg1), reg8(reg2), reg8(reg3));
      
      info.length = 3;
      break;
    }
      
    case OPCODE_ALU_NN: { info = {fmt::sprintf("%s %s, %s, %.2Xh", aluName(alu), reg8(reg1), reg8(reg2), unsigned8), 3}; break;}
    case OPCODE_ALU_NNNN: { info = {fmt::sprintf("%s %s, %s, %.4Xh", aluName((AluOp)(alu & 0b11110)), reg16(reg1), reg16(reg2), short2), 4}; break; }

    case OPCODE_JMP_NNNN:
    case OPCODE_JMPC_NNNN:
    {
      return {fmt::sprintf("%s%s %.4Xh", m[N_JMP], condName(cond), short1), 3};
    }
      
    case OPCODE_JMP_PP:
    case OPCODE_JMPC_PP:
    {
      return {fmt::sprintf("%s%s %s", m[N_JMP], condName(cond), reg16(reg2)), 2};
    }
      
    case OPCODE_NOP: { return {m[N_NOP], 1}; }
    
    case OPCODE_PUSH: { return {fmt::sprintf("%s %s", m[N_PUSH], reg8(reg1)), 1}; }
    case OPCODE_POP: { return {fmt::sprintf("%s %s", m[N_POP], reg8(reg1)), 1}; }
      
    case OPCODE_PUSH16: { return {fmt::sprintf("%s %s", m[N_PUSH], reg16(reg1)), 1}; }
    case OPCODE_POP16: { return {fmt::sprintf("%s %s", m[N_POP], reg16(reg1)), 1}; }
      
    case OPCODE_RET:
    case OPCODE_RETC:
    {
      return {fmt::sprintf("%s%s", m[N_RET], condName(cond)), 1};
    }
      
    case OPCODE_CALL:
    case OPCODE_CALLC:
    {
      return {fmt::sprintf("%s%s %.4Xh", m[N_CALL], condName(cond), short1), 3};
    }
      
    case OPCODE_LF: { return {fmt::sprintf("%s %s", m[N_LF], reg8(reg1)), 1}; }
    case OPCODE_SF: { return {fmt::sprintf("%s %s", m[N_SF], reg8(reg1)), 1}; }
      
    case OPCODE_EI: { return {"EI", 1}; }
    case OPCODE_DI: { return {"DI", 1}; }
      
    case OPCODE_SEXT: { return {fmt::sprintf("%s %s", m[N_SEXT], reg8(reg1)), 1}; }
      
    case OPCODE_CMP_REG:
    {
      bool extended = (alu & 0b1) != 0;
      
      if (extended)
        info.value = fmt::sprintf("%s %s, %s", m[N_CMP], reg16(reg1), reg16(reg2));
      else
        info.value = fmt::sprintf("%s %s, %s", m[N_CMP], reg8(reg1), reg8(reg2));
      
      info.length = 2;
      break;
    }
      
    case OPCODE_CMP_NN: { return {fmt::sprintf("%s %s, %.2Xh", m[N_CMP], reg8(reg1), unsigned8), 3}; break; }
    case OPCODE_CMP_NNNN: { return {fmt::sprintf("%s %s, %.4Xh", m[N_CMP], reg16(reg1), short2), 4}; break; }
  }
  
  return info;
}