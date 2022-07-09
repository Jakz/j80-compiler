#pragma once

#include "utils.h"
#include "instruction.h"

namespace Assembler
{
  struct op
  {
  private:
    const uint8_t data[4];

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

    u32 encoding() const
    {
      switch (length())
      {
        case 1: return data[0];
        case 2: return data[0] << 8 | data[1];
        case 3: return data[0] << 16 | data[1] << 8 | data[2];
        case 4: return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
        default: return data[0];
      }
    }

    s32 length() const
    {
      s32 lengths[32] = {
        1, 0, 0, 0, 3, 3, 4, 0, 1, 0, 1, 0, 2, 3, 4, 1, 2, 3, 4, 1, 3, 3, 3, 3, 3, 3, 2, 2, 1, 1, 3, 3
      };

      return lengths[opcode()];
    }

    std::string mnemonic() const;
  };
  
  class J80Disassembler
  {
  private:
    
  public:
    Instruction* disassemble(const byte* current);
  };
}
