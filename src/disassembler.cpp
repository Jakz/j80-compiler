#include "disassembler.h"

using namespace Assembler;

struct op
{
private:
  const byte data[4];
  
  static constexpr u8 OPCODE_MASK = 0x1F;
  static constexpr u8 OPCODE_SHIFT = 3;
  
  static constexpr u8 REG_MASK = 0x07;
  static constexpr u8 REG1_SHIFT = 0;
  static constexpr u8 REG2_SHIFT = 5;
  static constexpr u8 REG3_SHIFT = 5;

  
  static constexpr u8 CONDITION_MASK = 0x07;
  static constexpr u8 CONDITION_SHIFT = 0;
  
  static constexpr u8 ALU_MASK = 0x1F;
  static constexpr u8 ALU_SHIFT = 0;
  
public:
  
  Opcode opcode() const { return static_cast<Opcode>((data[0] >> OPCODE_SHIFT) & OPCODE_MASK); }
  Reg reg1() const { return static_cast<Reg>((data[0] >> REG1_SHIFT) & REG_MASK); }
  Reg reg2() const { return static_cast<Reg>((data[1] >> REG2_SHIFT) & REG_MASK); }
  Reg reg3() const { return static_cast<Reg>((data[2] >> REG3_SHIFT) & REG_MASK); }
  JumpCondition condition() const { return static_cast<JumpCondition>((data[0] >> CONDITION_SHIFT) & CONDITION_MASK); }
  Alu alu() const { return static_cast<Alu>((data[1] >> ALU_SHIFT) & ALU_MASK); }
  
  u8 uint8() const { return data[2]; }
  s8 sint8() const { return uint8(); }
  
  u16 uint16() const { return (data[1] << 8) | data[2]; }
  s16 sint16() const { return uint16(); }
  
  u16 uint16h() const { return (data[3] << 8) | data[2]; }
  s16 sint16h() const { return uint16(); }
};

const InstructionLength lengths[] = {
  LENGTH_1_BYTES, // 00000 NOP
  LENGTH_0_BYTES, // 00001 INVALID
  LENGTH_0_BYTES, // 00010 INVALID
  LENGTH_0_BYTES, // 00011 INVALID
  LENGTH_3_BYTES, // 00100 ALU R, S, Q
  LENGTH_3_BYTES, // 00101 ALU R, S, NN
  LENGTH_4_BYTES, // 00110 ALU P, Q, NNNN
  LENGTH_0_BYTES, // 00111 INVALID
  LENGTH_1_BYTES, // 01000 LF R
  LENGTH_0_BYTES, // 01001 INVALID
  LENGTH_1_BYTES, // 01010 SF R
  LENGTH_0_BYTES, // 01011 INVALID
  LENGTH_2_BYTES, // 01100 CMP R, S
  LENGTH_3_BYTES, // 01101 CMP R, NN
  LENGTH_4_BYTES, // 01110 CMP P, NNNN
  LENGTH_1_BYTES, // 01111 POP R
  LENGTH_2_BYTES, // 10000 LD/LSH/RSH R, S
  LENGTH_3_BYTES, // 10001 LD R, NN
  LENGTH_4_BYTES, // 10010 LD P, NNNN
  LENGTH_1_BYTES, // 10011 PUSH R
  LENGTH_3_BYTES, // 10100 LD R, [NNNN]
  LENGTH_3_BYTES, // 10101 LD R, [PP+SS]
  LENGTH_3_BYTES, // 10110 SD [NNNN], R
  LENGTH_3_BYTES, // 10111 SD [PP+SS], R
  LENGTH_3_BYTES, // 11000 JMPC NNNN
  LENGTH_3_BYTES, // 11001 JMP NNNN
  LENGTH_2_BYTES, // 11010 JMPC PP
  LENGTH_2_BYTES, // 11011 JMP PP
  LENGTH_1_BYTES, // 11100 RETC
  LENGTH_1_BYTES, // 11101 RET
  LENGTH_3_BYTES, // 11110 CALLC NNNN
  LENGTH_3_BYTES, // 11111 CALL NNNN
};

Instruction* J80Disassembler::disassemble(const byte* current)
{
  const op* data = reinterpret_cast<const op*>(current);
  InstructionLength length = lengths[data->opcode()];
  
  switch (data->opcode())
  {
    case OPCODE_NOP: return new InstructionNOP();
    case OPCODE_ALU_REG:
    {
      return new InstructionALU_R(data->reg1(), data->reg2(), data->reg3(), data->alu());
    }
    
  }
  
  
  return nullptr;
}
