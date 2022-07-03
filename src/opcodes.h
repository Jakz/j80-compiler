#ifndef _OPCODES_H_
#define _OPCODES_H_

#include "utils.h"

enum class Reg : u8
{
  A = 0b000,
  D = 0b001,
  F = 0b010,
  Y = 0b011,
  B = 0b100,
  C = 0b101,
  E = 0b110,
  X = 0b111,
  
  BA = 0b000,
  CD = 0b001,
  EF = 0b010,
  XY = 0b011,
  SP = 0b100,
  FP = 0b101,
  IX = 0b110,
  IY = 0b111
};

template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0> inline T operator|(const T& t, const Reg& o) { return t | static_cast<T>(o); }
template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0> inline T operator|(const Reg& o, const T& t) { return t | static_cast<T>(o); }
template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0> inline T operator<<(const Reg& o, const T& t) { return static_cast<T>(o) << t; }

enum class Alu : u8
{
  ADD8 = 0b10000,
  ADD16 = 0b10001,
  ADC8 = 0b10010,
  ADC16 = 0b10011,
  SUB8 = 0b10100,
  SUB16 = 0b10101,
  SBC8 = 0b10110,
  SBC16 = 0b101111,
  
  AND8 = 0b11000,
  AND16 = 0b11001,
  OR8 = 0b11010,
  OR16 = 0b11011,
  XOR8 = 0b11100,
  XOR16 = 0b11101,
  NOT8 = 0b11110,
  NOT16 = 0b11111,
  
  TRANSFER_A8 = 0b00010,
  TRANSFER_A16 = 0b00011,
  
  TRANSFER_B8 = 0b00100,
  TRANSFER_B16 = 0b00101,
  
  ADD_NO_FLAGS = 0b00111,
  
  LSH8 = 0b01100,
  LSH16 = 0b01101,
  RSH8 = 0b01110,
  RSH16 = 0b01111,
  
  SF = 0b01010,
  LF = 0b01000,
  
  EXTENDED_BIT = 0b00001
};

template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0> inline T operator==(const T& t, const Alu& o) { return t == static_cast<T>(o); }
template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0> inline T operator|(const T& t, const Alu& o) { return t | static_cast<T>(o); }
inline Alu operator|(const Alu& alu, const int& v) { return static_cast<Alu>(static_cast<int>(alu) | v); }
inline Alu operator|(const Alu& alu, const Alu& v) { return static_cast<Alu>(static_cast<int>(alu) | static_cast<int>(v)); }

inline Alu operator&(const Alu& alu, const int& v) { return static_cast<Alu>(static_cast<int>(alu) & v); }
inline bool operator&&(const Alu& alu, const Alu& f) { return ((std::underlying_type_t<Alu>)alu & (std::underlying_type_t<Alu>)f) == (std::underlying_type_t<Alu>)f; }


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
  

struct MnemonicInfo
{
  std::string value;
  u8 length;
};

class Opcodes
{
  private:

  public:
    static MnemonicInfo printInstruction(const u8 *data);
    //static void printInstruction(Instruction &i);
  
  static const char* opcodeName(Opcode opcode);
  static const char* reg(Reg reg, bool extended) { return extended ? reg16(reg) : reg8(reg); }
  static const char* reg8(Reg reg);
  static const char* reg16(Reg reg);
  static const char* aluName(Alu alu);
  static const char* condName(JumpCondition cond);
};

#endif
