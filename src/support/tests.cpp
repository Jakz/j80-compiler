#define CATCH_CONFIG_MAIN
#include "support/catch.hpp"

#include "instruction.h"

#include <array>

constexpr int OP_SHIFT = 3;
constexpr int REG2_SHIFT = 5;

using namespace Assembler;

using instruction_ptr = std::unique_ptr<Instruction>;

static const std::array<Reg, 8> regs8 = {
  Reg::A, Reg::D, Reg::F, Reg::Y,
  Reg::B, Reg::C, Reg::E, Reg::X
};

static const std::array<Reg, 8> regs16 = {
  Reg::BA, Reg::CD, Reg::EF, Reg::XY,
  Reg::SP, Reg::FP, Reg::IX, Reg::IY
};


struct assembled_instruction
{
private:
  u8 length;
  std::array<u8, 4> data;
  
public:
  assembled_instruction(Opcode op) : data({(u8)(op << OP_SHIFT), 0, 0, 0}), length(1) { }
  
  template<typename X> assembled_instruction(Opcode op, X x) :
    data({(op << OP_SHIFT) | (u8)x,0,0,0}), length(1) { }
  template<typename X, typename Y> assembled_instruction(Opcode op, X x, Y y) :
    data({(u8)((op << OP_SHIFT) | x),(u8)y,0,0}), length(2) { }
  template<typename X, typename Y, typename Z> assembled_instruction(Opcode op, X x, Y y, Z z) :
    data({((u8)(op << OP_SHIFT) | x),(u8)y,(u8)z,0}), length(3) { }
  template<typename X, typename Y, typename Z, typename K> assembled_instruction(Opcode op, X x, Y y, Z z, K k) :
    data({(u8)((op << OP_SHIFT) | x),(u8)y,(u8)z,(u8)k}), length(4) { }
  
  bool operator==(const u8 (&d) [4]) const
  {
    return memcmp(data.data(), d, length) == 0;
  }
  
  bool operator==(const instruction_ptr& i) const
  {
    u8 data[4];
    i->assemble(data);
    return i->getLength() == length && *this == data;
  }
};

#define CHECK1(x) REQUIRE(data[0] == (x))
#define CHECK2(x, y) REQUIRE((data[0] == (x)) && (data[1] == (y)))
#define CHECK3(x, y, z) (data[0] == (x)) && (data[1] == (y)) && (data[2] == (z))

TEST_CASE("instructions are correctly generated", "[instruction gen]")
{
  u8 data[4];
  
  SECTION("NOP")
  {
    assembled_instruction ai(OPCODE_NOP);
    instruction_ptr i(new InstructionNOP());
    REQUIRE(ai == i);
  }
  
  SECTION("LD R, S")
  {
    for (const auto r : regs8)
    {
      for (const auto s : regs8)
      {
        assembled_instruction ai(OPCODE_LD_RSH_LSH, r, (s << REG2_SHIFT) | Alu::TRANSFER_A8);
        instruction_ptr i(new InstructionLD_LSH_RSH(r, s, Alu::TRANSFER_A8, false));
        REQUIRE(ai == i);
      }
    }
  }
  
  SECTION("LSH R, S")
  {
    for (const auto r : regs8)
    {
      for (const auto s : regs8)
      {
        assembled_instruction ai(OPCODE_LD_RSH_LSH, r, (s << REG2_SHIFT) | Alu::LSH8);
        instruction_ptr i(new InstructionLD_LSH_RSH(r, s, Alu::LSH8, false));
        REQUIRE(ai == i);
      }
    }
  }
  
  SECTION("RSH R, S")
  {
    for (const auto r : regs8)
    {
      for (const auto s : regs8)
      {
        assembled_instruction ai(OPCODE_LD_RSH_LSH, r, (s << REG2_SHIFT) | Alu::RSH8);
        instruction_ptr i(new InstructionLD_LSH_RSH(r, s, Alu::RSH8, false));
        REQUIRE(ai == i);
      }
    }
  }
  
  SECTION("LD P, Q")
  {
    for (const auto r : regs16)
    {
      for (const auto s : regs16)
      {
        assembled_instruction ai(OPCODE_LD_RSH_LSH, r, (s << REG2_SHIFT) | Alu::TRANSFER_A16);
        instruction_ptr i(new InstructionLD_LSH_RSH(r, s, Alu::TRANSFER_A8, true));
        REQUIRE(ai == i);
      }
    }
  }
  
  SECTION("LSH P, Q")
  {
    for (const auto r : regs16)
    {
      for (const auto s : regs16)
      {
        assembled_instruction ai(OPCODE_LD_RSH_LSH, r, (s << REG2_SHIFT) | Alu::LSH16);
        instruction_ptr i(new InstructionLD_LSH_RSH(r, s, Alu::LSH16, true));
        REQUIRE(ai == i);
      }
    }
  }
  
  SECTION("RSH P, Q")
  {
    for (const auto r : regs16)
    {
      for (const auto s : regs16)
      {
        assembled_instruction ai(OPCODE_LD_RSH_LSH, r, (s << REG2_SHIFT) | Alu::LSH16);
        instruction_ptr i(new InstructionLD_LSH_RSH(r, s, Alu::LSH16, true));
        REQUIRE(ai == i);
      }
    }
  }
  
  SECTION("LD R, NN")
  {
    for (const auto r : regs8)
    {
      for (u32 n = 0; n < 256; ++n)
      {
        assembled_instruction ai(OPCODE_LD_NN, r, Alu::TRANSFER_B8, n);
        instruction_ptr i(new InstructionLD_NN(r, n));
        REQUIRE(ai == i);
      }
    }
  }

  SECTION("LD P, NNNN")
  {
    for (const auto r : regs16)
    {
      for (u32 n = 0; n < 65536; ++n)
      {
        assembled_instruction ai(OPCODE_LD_NNNN, r, (n >> 8) & 0xFF, n & 0xFF);
        instruction_ptr i(new InstructionLD_NNNN(r, n));
        REQUIRE(ai == i);
      }
    }
  }
  
  /*SECTION("LD R, [NNNN]")
  {
    for (const auto r : regs8)
    {
      for (u32 n = 0; n < 65536; ++n)
      {
        assembled_instruction ai(OPCODE_LD_PTR_NNNN, r, (n >> 8) & 0xFF, n & 0xFF);
        instruction_ptr i(new InstructionLD_NNNN(r, n));
        REQUIRE(ai == i);
      }
    }
  }*/
}
