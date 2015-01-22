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
      u8 B;
      u8 A;
    };
    
    u16 BA;
  };
  union
  {
    struct
    {
      u8 C;
      u8 D;
    };
    
    u16 CD;
  };
  union
  {
    struct
    {
      u8 X;
      u8 Y;
    };
    
    u16 XY;
  };
  union
  {
    struct
    {
      u8 E;
      u8 F;
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
  CARRY = 0x01,
  ZERO = 0x02,
  SIGN = 0x04,
  OVERFLOW = 0x08
};

class VM
{
  private:
    Regs regs;
    u8 *code;
    u8 *ram;
    bool interruptEnabled;
  
    void add8(u8& op1, u8& op2, u8& dest, bool flags = true);
    void adc8(u8& op1, u8& op2, u8& dest, bool flags = true);
    void sub8(u8& op1, u8& op2, u8& dest, bool flags = true);
    void sbc8(u8& op1, u8& op2, u8& dest, bool flags = true);
    void add16(u16& op1, u16& op2, u16& dest, bool flags = true);
    void adc16(u16& op1, u16& op2, u16& dest, bool flags = true);
    void sub16(u16& op1, u16& op2, u16& dest, bool flags = true);
    void sbc16(u16& op1, u16& op2, u16& dest, bool flags = true);
  
    inline void setFlag(Flag flag) { regs.FLAGS |= flag; }
    inline void unsetFlag(Flag flag) { regs.FLAGS &= ~flag; }
  
  public:
    void reset() { regs.PC = 0; }
    void executeInstruction();
  
    bool isConditionTrue(JumpCondition condition) const;
  
    void ramWrite(u16 address, u8 value) { ram[address] = value; }
    u8 ramRead(u16 address) { return ram[address]; }
  
    u8& reg8(Reg r);
    u16& reg16(Reg r);
};


#endif