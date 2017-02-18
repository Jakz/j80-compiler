#ifndef _VM_H_
#define _VM_H_

#include "utils.h"

#include "opcodes.h"

struct Regs
{
  union
  {
    struct
    {
      u8 A;
      u8 B;
    };
    
    u16 BA;
  };
  union
  {
    struct
    {
      u8 D;
      u8 C;
    };
    
    u16 CD;
  };
  union
  {
    struct
    {
      u8 Y;
      u8 X;
    };
    
    u16 XY;
  };
  union
  {
    struct
    {
      u8 F;
      u8 E;
    };
    
    u16 EF;
  };
  
  u16 SP;
  u16 FP;
  u16 IX;
  u16 IY;
  
  u8 FLAGS;
  
  u16 PC;
};

enum Flag : u8
{
  FLAG_CARRY = 0x01,
  FLAG_ZERO = 0x02,
  FLAG_SIGN = 0x04,
  FLAG_OVERFLOW = 0x08
};

class StdOut
{
public:
  virtual void out(u8 value) = 0;
};

class VM
{
  private:
    StdOut* sout;
    Regs regs;
    u8 *memory;
    bool interruptEnabled;
  
    template <typename W> void add(const W& op1, const W& op2, W& dest, bool flags = true);
    template <typename W> void adc(const W& op1, const W& op2, W& dest, bool flags = true);
    template <typename W> void sub(const W& op1, const W& op2, W& dest, bool flags = true);
    template <typename W> void sbc(const W& op1, const W& op2, W& dest, bool flags = true);
    template <typename W> void alu(AluOp op, const W& op1, const W& op2, W& dest, bool flags = true);
    template <typename W> void aluFlagsArithmetic(const W& op1, const W& op2, const W& dest);
  
    template <typename W> bool isNegative(W& value) { return (value & (1 << (sizeof(W)*8-1))) != 0; }

    inline void setFlag(Flag flag, bool value) { if (value) setFlag(flag); else unsetFlag(flag); }
    inline void setFlag(Flag flag) { regs.FLAGS |= flag; }
    inline void unsetFlag(Flag flag) { regs.FLAGS &= ~flag; }
    inline bool isFlagSet(Flag flag) { return (regs.FLAGS & flag) != 0; }
  
  public:
    VM() { reset(); memory = new u8[0xFFFF]; }
    ~VM() { delete[] memory; }
    void reset() { memset(&regs, 0, sizeof(Regs)); }
    void executeInstruction();
  
    void setStdOut(StdOut* out) { this->sout = out; }
  
    bool isConditionTrue(JumpCondition condition) const;
  
    void copyToRam(u8* data, size_t length, u16 offset = 0)
    {
      memcpy(&memory[offset], data, length);
    }
  
    void ramWrite(u16 address, u8 value);
    u8 ramRead(u16 address) { return memory[address]; }
    const u8* ram() { return memory; }

    u16 pc() { return regs.PC; }
    u8 flags() { return regs.FLAGS; }
    u8& reg8(Reg r);
    u16& reg16(Reg r);
};


#endif
