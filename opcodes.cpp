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

const char* Opcodes::aluName(AluOp alu)
{
  switch (alu)
  {
    case ALU_ADD8: return "ADD";
    case ALU_ADC8: return "ADC";
    case ALU_SUB8: return "SUB";
    case ALU_SBC8: return "SBC";
    case ALU_AND8: return "AND";
    case ALU_OR8: return "OR";
    case ALU_XOR8: return "XOR";
    case ALU_NOT8: return "NOT";
    default: { printf("ERRORE ALU NAME! %d\n", alu); return "ERR"; }
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

int Opcodes::printInstruction(u8 *data, char buffer[64])
{
  u8 *d = data;
  
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

  
  switch (opcode) {
    case OPCODE_LD_RSH_LSH:
    {
      if (alu == ALU_TRANSFER_A8)
        sprintf(buffer, "LD %s, %s", reg8(reg1), reg8(reg2) );
      else if (alu == ALU_TRANSFER_A16)
        sprintf(buffer, "LD %s, %s", reg16(reg1), reg16(reg2) );
      else if (alu == ALU_LSH8)
        sprintf(buffer, "LSH %s", reg8(reg1));
      else if (alu == ALU_RSH8)
        sprintf(buffer, "RSH %s", reg8(reg1));
      else if (alu == ALU_LSH16)
        sprintf(buffer, "LSH %s", reg16(reg1));
      else if (alu == ALU_RSH16)
        sprintf(buffer, "RSH %s", reg16(reg1));
      return 2;
    }
      
    case OPCODE_LD_NN: { sprintf(buffer, "LD %s, %.2Xh", reg8(reg1), unsigned8); return 3; }
    case OPCODE_LD_NNNN: { sprintf(buffer, "LD %s, %.4Xh", reg16(reg1), short1); return 3; }
    case OPCODE_LD_PTR_NNNN: { sprintf(buffer, "LD %s, [%.4Xh]", reg8(reg1), short1); return 3; }
    case OPCODE_LD_PTR_PP: { sprintf(buffer, "LD %s, [%s%+d]", reg8(reg1), reg16(reg2), signed8); return 3; }
      
    case OPCODE_SD_PTR_NNNN: { sprintf(buffer, "ST %s, [%.4Xh]", reg8(reg1), short1); return 3; }
    case OPCODE_SD_PTR_PP: { sprintf(buffer, "ST %s, [%s%+d]", reg8(reg1), reg16(reg2), signed8); return 3; }

    case OPCODE_ALU_REG:
    {
      bool extended = (alu & 0b1) != 0;
      
      if (extended)
        sprintf(buffer, "%s %s, %s, %s", aluName((AluOp)(alu & 0b11110)), reg16(reg1), reg16(reg2), reg16(reg3));
      else
        sprintf(buffer, "%s %s, %s, %s", aluName(alu), reg8(reg1), reg8(reg2), reg8(reg3));
      
      return 3;
    }
      
    case OPCODE_ALU_NN: { sprintf(buffer, "%s %s, %s, %.2Xh", aluName(alu), reg8(reg1), reg8(reg2), unsigned8); return 3; }
    case OPCODE_ALU_NNNN: { sprintf(buffer, "%s %s, %s, %.4Xh", aluName((AluOp)(alu & 0b11110)), reg16(reg1), reg16(reg2), short2); return 4; }

    case OPCODE_JMP_NNNN:
    case OPCODE_JMPC_NNNN:
    {
      sprintf(buffer, "JMP%s %.4Xh", condName(cond), short1);
      return 3;
    }
      
    case OPCODE_JMP_PP:
    case OPCODE_JMPC_PP:
    {
      sprintf(buffer, "JMP%s %s", condName(cond), reg16(reg2));
      return 2;
    }
      
    case OPCODE_NOP: { sprintf(buffer, "NOP"); return 1; }
    
    case OPCODE_PUSH: { sprintf(buffer, "PUSH %s", reg8(reg1)); return 1; }
    case OPCODE_POP: { sprintf(buffer, "POP %s", reg8(reg1)); return 1; }
      
    case OPCODE_RET:
    case OPCODE_RETC:
    {
      sprintf(buffer, "RET%s", condName(cond));
      return 1;
    }
      
    case OPCODE_CALL:
    case OPCODE_CALLC:
    {
      sprintf(buffer, "CALL%s %.4Xh", condName(cond), short1);
      return 3;
    }
      
    case OPCODE_LF: { sprintf(buffer, "LF %s", reg8(reg1)); return 1; }
    case OPCODE_SF: { sprintf(buffer, "SF %s", reg8(reg1)); return 1; }
      
    case OPCODE_EI: { sprintf(buffer, "EI"); return 1; }
    case OPCODE_DI: { sprintf(buffer, "DI"); return 1; }
    case OPCODE_INT: { sprintf(buffer, "INT"); return 1; }
      
    case OPCODE_CMP_REG:
    {
      bool extended = (alu & 0b1) != 0;
      
      if (extended)
        sprintf(buffer, "CMP %s, %s", reg16(reg1), reg16(reg2));
      else
        sprintf(buffer, "CMP %s, %s", reg8(reg1), reg8(reg2));
      
      return 2;
    }
      
    case OPCODE_CMP_NN: { sprintf(buffer, "CMP %s, %.2Xh", reg8(reg1), unsigned8); return 3; }
    case OPCODE_CMP_NNNN: { sprintf(buffer, "CMP %s, %.4Xh", reg16(reg1), short2); return 4; }

      
    default:
      break;
  }
  
  
}