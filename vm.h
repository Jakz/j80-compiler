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

class VM
{
  private:
    Regs regs;
    u8 *code;
    u8 *ram;
  
  public:
    void reset() { regs.PC = 0; }
    void executeInstruction();
  
    void ramWrite(u16 address, u8 value) { ram[address] = value; }
    u8 ramRead(u16 address) { return ram[address]; }
  
    u8& reg8(Reg r);
    u16& reg16(Reg r);
};


#endif