#ifndef _VM_H_
#define _VM_H_

#include "utils.h"

#include "opcodes.h"

enum Flag : u8
{
  FLAG_CARRY    = 0b0001,
  FLAG_ZERO     = 0b0010,
  FLAG_SIGN     = 0b0100,
  FLAG_OVERFLOW = 0b1000,
};

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

  u8& reg8(Reg r)
  {
    switch (r) {
      case Reg::A: return A;
      case Reg::B: return B;
      case Reg::C: return C;
      case Reg::D: return D;
      case Reg::E: return E;
      case Reg::F: return F;
      case Reg::X: return X;
      case Reg::Y: return Y;
    }
  }

  u16& reg16(Reg r)
  {
    switch (r) {
      case Reg::BA: return BA;
      case Reg::CD: return CD;
      case Reg::EF: return EF;
      case Reg::XY: return XY;
      case Reg::SP: return SP;
      case Reg::FP: return FP;
      case Reg::IX: return IX;
      case Reg::IY: return IY;
    }
  }

  bool flag(Flag f) { return (FLAGS & f) == f; }
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

    u32 dataSegmentStart;
  
    template <typename W> void add(const W& op1, const W& op2, W& dest, bool flags = true);
    template <typename W> void adc(const W& op1, const W& op2, W& dest, bool flags = true);
    template <typename W> void sub(const W& op1, const W& op2, W& dest, bool flags = true);
    template <typename W> void sbc(const W& op1, const W& op2, W& dest, bool flags = true);
    template <typename W> void alu(Alu op, const W& op1, const W& op2, W& dest, bool result, bool flags);
  
    template <typename W> bool isNegative(W& value) { return (value & (1 << (sizeof(W)*8-1))) != 0; }

    inline void setFlag(Flag flag, bool value) { if (value) setFlag(flag); else unsetFlag(flag); }
    inline void setFlag(Flag flag) { regs.FLAGS |= flag; }
    inline void unsetFlag(Flag flag) { regs.FLAGS &= ~flag; }
    inline bool isFlagSet(Flag flag) { return (regs.FLAGS & flag) != 0; }
  
  public:
    VM()
    {
      reset();
      memory = new u8[0xFFFF]; 
      dataSegmentStart = 0xFFFFFFFF;
    }

    ~VM() { delete[] memory; }

    void reset() { memset(&regs, 0, sizeof(Regs)); }
    void executeInstruction();
  
    void setStdOut(StdOut* out) { this->sout = out; }
    void setDataSegmentStart(u32 dss) { this->dataSegmentStart = dss; }

    u32 getDataSegmentStart() { return dataSegmentStart; }
  
    bool isConditionTrue(JumpCondition condition) const;
  
    void copyToRam(u8* data, size_t length, u16 offset = 0)
    {
      memcpy(&memory[offset], data, length);
    }
  
    void ramWrite(u16 address, u8 value);
    u8 ramRead(u16 address) const { return memory[address]; }
    const u8* ram() { return memory; }

    auto& allRegs() { return regs; }

    u16 pc() const { return regs.PC; }
    u8 flags() const { return regs.FLAGS; }
    u8& reg8(Reg r) { return regs.reg8(r); }
    u16& reg16(Reg r) { return regs.reg16(r); }

    void run()
    {
      while (true)
        executeInstruction();
    }
};


#endif
