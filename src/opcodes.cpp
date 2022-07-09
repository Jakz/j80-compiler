#include "opcodes.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "assembler.h"

const char* Opcodes::reg8(Reg reg)
{
  switch (reg)
  {
    case Reg::A: return "A";
    case Reg::B: return "B";
    case Reg::X: return "X";
    case Reg::Y: return "Y";
    case Reg::C: return "C";
    case Reg::D: return "D";
    case Reg::E: return "E";
    case Reg::F: return "F";

    default:
      assert(false);
      return nullptr;
  }
}

const char* Opcodes::reg16(Reg reg)
{
  switch (reg)
  {
    case Reg::BA: return "BA";
    case Reg::XY: return "XY";
    case Reg::CD: return "CD";
    case Reg::EF: return "EF";
    case Reg::SP: return "SP";
    case Reg::FP: return "FP";
    case Reg::IX: return "IX";
    case Reg::IY: return "IY";
      
    default:
      assert(false);
      return nullptr;
  }
}

const char* Opcodes::condName(JumpCondition cond)
{
  switch (cond)
  {
    case COND_UNCOND: return "";
    case COND_CARRY: return "c";
    case COND_NCARRY: return "nc";
    case COND_ZERO: return "z";
    case COND_NZERO: return "nz";
    case COND_OVERFLOW: return "v";
    case COND_NOVERFLOW: return "nv";
    case COND_SIGN: return "n";
    case COND_NSIGN: return "nn";
      
    default:
      assert(false);
      return nullptr;
  }
}

const char* Opcodes::aluName(Alu alu)
{
  alu = static_cast<Alu>(alu & 0b11110);
  
  switch (alu)
  {
    case Alu::ADD8: return "add";
    case Alu::ADC8: return "adc";
    case Alu::SUB8: return "sub";
    case Alu::SBC8: return "sbc";
    case Alu::AND8: return "and";
    case Alu::OR8: return "or";
    case Alu::XOR8: return "xor";
    case Alu::NOT8: return "not";
      
    case Alu::LSH8:
    case Alu::LSH16:
      return "lsh";
      
    case Alu::RSH8:
    case Alu::RSH16:
      return "rsh";
      
    case Alu::TRANSFER_A8: return "ld";
    case Alu::TRANSFER_A16: return "ld";
      
    default:
      assert(false);
      return nullptr;
  }
}

const char* Opcodes::opcodeName(Opcode opcode)
{
  switch (opcode)
  {
    case OPCODE_RET:
    case OPCODE_RETC:
      return "ret";
      

    case OPCODE_CALL:
    case OPCODE_CALLC:
      return "call";
      
    case OPCODE_POP:
    case OPCODE_POP16:
      return "pop";
    
    case OPCODE_PUSH:
    case OPCODE_PUSH16:
      return "push";
      
    case OPCODE_ALU_NN:
    case OPCODE_ALU_REG:
    case OPCODE_ALU_NNNN:
      return "";
      
    case OPCODE_CMP_NN:
    case OPCODE_CMP_REG:
    case OPCODE_CMP_NNNN:
      return "cmp";

    case OPCODE_JMP_PP:
    case OPCODE_JMPC_PP:
    case OPCODE_JMP_NNNN:
    case OPCODE_JMPC_NNNN:
      return "jmp";
      
    case OPCODE_LD_NN:
    case OPCODE_LD_NNNN:
    case OPCODE_LD_PTR_PP:
    case OPCODE_LD_RSH_LSH:
    case OPCODE_LD_PTR_NNNN:
      return "ld";
      
    case OPCODE_SD_PTR_PP:
    case OPCODE_SD_PTR_NNNN:
      return "st";
      
    case OPCODE_DI: return "di";
    case OPCODE_EI: return  "ei";
    case OPCODE_LF: return  "lf";
    case OPCODE_SF: return  "sf";
    case OPCODE_NOP: return "nop";
    case OPCODE_SEXT: return "sext";
      
    default:
      assert(false);
      return nullptr;
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
  Alu alu = static_cast<Alu>(d[1] & 0b11111);
  JumpCondition cond = static_cast<JumpCondition>(d[0] & 0b1111);

  MnemonicInfo info = {"N/A", 0};
  
  switch (opcode) {
      
    case OPCODE_LD_NN: { info = {fmt::format("{} {}, {:2X}h", opcodeName(opcode), reg8(reg1), unsigned8), 3}; break; }
    case OPCODE_LD_NNNN: { info = {fmt::format("{} {}, {:04X}h", opcodeName(opcode), reg16(reg1), short1), 3}; break; }
    case OPCODE_LD_PTR_NNNN: { info = {fmt::format("{} {}, [{:04X}h]",opcodeName(opcode), reg8(reg1), short1), 3}; break; }
    case OPCODE_LD_PTR_PP: { info = {fmt::format("{} {}, [{}{:+d}]", opcodeName(opcode), reg8(reg1), reg16(reg2), signed8), 3}; break; }
      
    case OPCODE_SD_PTR_NNNN: { info = {fmt::format("{} [{:04X}h], {} ", opcodeName(opcode), short1, reg8(reg1)), 3}; break; }
    case OPCODE_SD_PTR_PP: { info = {fmt::format("{} [{}{:+d}], {} ", opcodeName(opcode), reg16(reg2), signed8, reg8(reg1)), 3}; break; }

    case OPCODE_JMP_NNNN:
    case OPCODE_JMPC_NNNN:
    {
      return {fmt::format("{}{} %.4Xh", opcodeName(opcode), condName(cond), short1), 3};
    }
      
    case OPCODE_JMP_PP:
    case OPCODE_JMPC_PP:
    {
      return {fmt::format("{}{} {}", opcodeName(opcode), condName(cond), reg16(reg2)), 2};
    }
      
    case OPCODE_NOP: { return {opcodeName(opcode), 1}; }
    
    case OPCODE_PUSH: { return {fmt::format("{} {}", opcodeName(opcode), reg8(reg1)), 1}; }
    case OPCODE_POP: { return {fmt::format("{} {}", opcodeName(opcode), reg8(reg1)), 1}; }
      
    case OPCODE_PUSH16: { return {fmt::format("{} {}", opcodeName(opcode), reg16(reg1)), 1}; }
    case OPCODE_POP16: { return {fmt::format("{} {}", opcodeName(opcode), reg16(reg1)), 1}; }
      
    case OPCODE_RET:
    case OPCODE_RETC:
    {
      return {fmt::format("{}{}", opcodeName(opcode), condName(cond)), 1};
    }
      
    case OPCODE_CALL:
    case OPCODE_CALLC:
    {
      return {fmt::format("{}{} {:04X}h", opcodeName(opcode), condName(cond), short1), 3};
    }
      
    case OPCODE_LF: { return {fmt::format("{} {}", opcodeName(opcode), reg8(reg1)), 1}; }
    case OPCODE_SF: { return {fmt::format("{} {}", opcodeName(opcode), reg8(reg1)), 1}; }
      
    case OPCODE_EI: { return {"EI", 1}; }
    case OPCODE_DI: { return {"DI", 1}; }
      
    case OPCODE_SEXT: { return {fmt::format("{} {}", opcodeName(opcode), reg8(reg1)), 1}; }
      
    case OPCODE_CMP_REG:
    {
      bool extended = (alu & 0b1) == Alu::EXTENDED_BIT;
      
      if (extended)
        info.value = fmt::format("{} {}, {}", opcodeName(opcode), reg16(reg1), reg16(reg2));
      else
        info.value = fmt::format("{} {}, {}", opcodeName(opcode), reg8(reg1), reg8(reg2));
      
      info.length = 2;
      break;
    }

    case OPCODE_ALU_NN: { return { fmt::format("{} {}, {}, {+d}", aluName(alu), reg8(reg1), reg8(reg2), signed8) }; break; }
    case OPCODE_ALU_NNNN: { return { fmt::format("{} {}, {}, {:04X}h", aluName(alu), reg16(reg1), reg16(reg2), short2) }; break; }
    case OPCODE_ALU_REG: 
    {
      if ((u8)alu & (u8)Alu::EXTENDED_BIT)
      {
        return { fmt::format("{} {}, {}, {}", aluName(alu), reg8(reg1), reg8(reg2), reg8(reg3)) }; break;
      }
      else
      {
        return { fmt::format("{} {}, {}, {}", aluName(alu), reg16(reg1), reg16(reg2), reg16(reg3)) }; break;
      }
    }

    case OPCODE_CMP_NN: { return {fmt::format("{} {}, {:02X}h", opcodeName(opcode), reg8(reg1), unsigned8), 3}; break; }
    case OPCODE_CMP_NNNN: { return {fmt::format("{} {}, {:04X}h", opcodeName(opcode), reg16(reg1), short2), 4}; break; }

    default: assert(false);
  }
  
  return info;
}
