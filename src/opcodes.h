#ifndef _OPCODES_H_
#define _OPCODES_H_

#include "utils.h"

enum Reg : u8
{
  REG_NONE = 0b000,
  
  REG_A = 0b000,
  REG_D = 0b001,
  REG_F = 0b010,
  REG_Y = 0b011,
  REG_B = 0b100,
  REG_C = 0b101,
  REG_E = 0b110,
  REG_X = 0b111,
  
  REG_BA = 0b000,
  REG_CD = 0b001,
  REG_EF = 0b010,
  REG_XY = 0b011,
  REG_SP = 0b100,
  REG_FP = 0b101,
  REG_IX = 0b110,
  REG_IY = 0b111
};

enum AluOp : u8
{
  ALU_ADD8 = 0b10000,
  ALU_ADD16 = 0b10001,
  ALU_ADC8 = 0b10010,
  ALU_ADC16 = 0b10011,
  ALU_SUB8 = 0b10100,
  ALU_SUB16 = 0b10101,
  ALU_SBC8 = 0b10110,
  ALU_SBC16 = 0b101111,
  
  ALU_AND8 = 0b11000,
  ALU_AND16 = 0b11001,
  ALU_OR8 = 0b11010,
  ALU_OR16 = 0b11011,
  ALU_XOR8 = 0b11100,
  ALU_XOR16 = 0b11101,
  ALU_NOT8 = 0b11110,
  ALU_NOT16 = 0b11111,
  
  ALU_TRANSFER_A8 = 0b00010,
  ALU_TRANSFER_A16 = 0b00011,
  
  ALU_TRANSFER_B8 = 0b00100,
  ALU_TRANSFER_B16 = 0b00101,
  
  ALU_ADD_NO_FLAGS = 0b00111,
  
  ALU_LSH8 = 0b01100,
  ALU_LSH16 = 0b01101,
  ALU_RSH8 = 0b01110,
  ALU_RSH16 = 0b01111,
  
  ALU_SF = 0b01010,
  ALU_LF = 0b01000,
};

const u8 BASE_OPCODE = 0b00001;

enum Opcode : u8
{
  OPCODE_LD_RSH_LSH = 0b10000,
  OPCODE_LD_NN = 0b10001,
  OPCODE_LD_NNNN = 0b10010,
  OPCODE_LD_PTR_NNNN = 0b10100,
  OPCODE_LD_PTR_PP = 0b10101,
  
  OPCODE_SD_PTR_NNNN = 0b10110,
  OPCODE_SD_PTR_PP = 0b10111,
  
  OPCODE_ALU_REG = 0b00100,
  OPCODE_ALU_NN = 0b00101,
  OPCODE_ALU_NNNN = 0b00110,
  
  OPCODE_JMP_NNNN = 0b11001,
  OPCODE_JMPC_NNNN = 0b11000,
  
  OPCODE_NOP = 0b00000,
  
  OPCODE_JMP_PP = 0b11011,
  OPCODE_JMPC_PP = 0b11010,
  
  OPCODE_PUSH = 0b10011,
  OPCODE_POP = 0b01111,
  
  OPCODE_PUSH16 = 0b01001,
  OPCODE_POP16 = 0b01011,
  
  OPCODE_RET = 0b11101,
  OPCODE_RETC = 0b11100,
  
  OPCODE_CALL = 0b11111,
  OPCODE_CALLC = 0b11110,
  
  OPCODE_LF = 0b01000,
  OPCODE_SF = 0b01010,
  
  OPCODE_CMP_REG = 0b01100,
  OPCODE_CMP_NN = 0b01101,
  OPCODE_CMP_NNNN = 0b01110,
  
  OPCODE_EI = 0b00010,
  OPCODE_DI = 0b00011,
  OPCODE_INT = 0b01011,
  
  OPCODE_SEXT = 0b00001
};

enum JumpCondition : u8
{
  COND_UNCOND = 0b1000,
  COND_CARRY = 0b0000,
  COND_ZERO = 0b0001,
  COND_SIGN = 0b0010,
  COND_OVERFLOW = 0b0011,
  COND_NCARRY = 0b0100,
  COND_NZERO = 0b0101,
  COND_NSIGN = 0b0110,
  COND_NOVERFLOW = 0b111
};
  

struct Instruction;

class Opcodes
{
  private:

  public:
    static int printInstruction(u8 *data);
    //static void printInstruction(Instruction &i);
  
  static const char* reg8(u8 reg);
  static const char* reg16(u8 reg);
  static const char* aluName(AluOp alu);
  static const char* condName(JumpCondition cond);
};

#endif