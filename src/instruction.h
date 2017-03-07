#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include <unordered_map>
#include <vector>
#include <list>

#include "support/format.h"
#include "opcodes.h"

namespace Assembler
{
  
  class J80Assembler;
  
  enum InstructionLength : u8
  {
    LENGTH_0_BYTES = 0,
    LENGTH_1_BYTES = 1,
    LENGTH_2_BYTES = 2,
    LENGTH_3_BYTES = 3,
    LENGTH_4_BYTES = 4
  };
  
#pragma mark Environment
  struct DataSegmentEntry
  {
    std::unique_ptr<u8[]> data;
    u16 length;
    u16 offset;
    
    DataSegmentEntry(DataSegmentEntry&& other) : data(std::move(other.data)), length(other.length), offset(other.offset)
    {
      
    }
    
    DataSegmentEntry(const DataSegmentEntry& other) : DataSegmentEntry(other.length, other.offset)
    {
      std::copy(other.data.get(), other.data.get()+length, data.get());
    }
    
    DataSegmentEntry() : data(nullptr), length(0), offset(0) { }
    
    DataSegmentEntry& operator=(const DataSegmentEntry& other)
    {
      this->data = std::unique_ptr<u8[]>(new u8[other.length]);
      this->length = other.length;
      this->offset = other.offset;
      std::copy(other.data.get(), other.data.get()+length, data.get());
      return *this;
    }
    
    DataSegmentEntry& operator=(DataSegmentEntry&& other)
    {
      this->data = std::move(other.data);
      this->length = other.length;
      this->offset = other.offset;
      return *this;
    }
    
    DataSegmentEntry(const std::string& ascii, bool includeNul) :
    DataSegmentEntry(includeNul ? ascii.length()+1 : ascii.length())
    {
      data.get()[length] = '\0';
      std::copy(ascii.begin(), ascii.end(), data.get());
    }
    
    DataSegmentEntry(const std::list<u8>& data) : DataSegmentEntry(data.size())
    {
      std::copy(data.begin(), data.end(), this->data.get());
    }
    
    DataSegmentEntry(const std::list<u16>& data) : DataSegmentEntry(data.size()*2)
    {
      size_t index = 0;
      std::for_each(data.begin(), data.end(), [&index, this] (u16 value) {
        this->data[index++] = value & 0xFF;
        this->data[index++] = (value & 0xFF00) >> 8;
      });
    }
    
    DataSegmentEntry(u16 size, u16 offset = 0) : data(new u8[size]), length(size), offset(offset) { }
    
    const u8* getData() const { return this->data.get(); }
  };
  
  struct DataReference
  {
    enum class Type
    {
      POINTER,
      LENGTH8,
      LENGTH16
    };
    
    Type type;
    std::string label;
    s8 offset;
    
    DataReference(const std::string& label, s8 offset) : type(Type::POINTER), label(label), offset(offset) { }
    DataReference(const std::string& label, Type type) : type(type), label(label), offset(0) { }
  };

  using InterruptIndex = u8;
  
  struct data_map
  {
    using map_t = std::unordered_map<std::string, DataSegmentEntry>;
    map_t map;
    std::vector<map_t::iterator> lru;
    
    void clear() { map.clear(); lru.clear(); }
    
    decltype(lru)::const_iterator begin() const { return lru.begin(); }
    decltype(lru)::const_iterator end() const { return lru.end(); }
  };
  
  using const_map = std::unordered_map<std::string, u16>;
  using assembler = J80Assembler;
  
  struct Environment
  {
    assembler& assembler;
    data_map& data;
    const_map& consts;
    u16 dataSegmentBase;
    
    Environment(J80Assembler& assemb, data_map& data, const_map& consts, u16 dataSegmentBase) :
    assembler(assemb), data(data), consts(consts), dataSegmentBase(dataSegmentBase) { }
  };
  
#pragma mark Support Types
  struct Address
  {
    enum Type
    {
      ABSOLUTE,
      LABEL,
      INTERRUPT
    } type;
    
    u16 address;
    std::string label;
    InterruptIndex interrupt;
    
    Address() : type(ABSOLUTE), address(0) { }
    Address(u16 address) : type(ABSOLUTE), address(address) { }
    Address(const std::string& label) : type(LABEL), label(label) { }
    Address(InterruptIndex interrupt) : type(INTERRUPT), interrupt(interrupt) { }
  };
  
  struct Value8
  {
    enum Type
    {
      VALUE,
      DATA_LENGTH,
      CONST
    } type;
    
    mutable u8 value;
    std::string label;
    
    Value8() = default;
    Value8(u8 value) : type(VALUE), value(value) { }
    Value8(Type type, const std::string& label) : type(type), label(label) { }
  };
  
  struct Value16
  {
    enum Type
    {
      VALUE,
      DATA_LENGTH,
      CONST,
      LABEL_ADDRESS
    } type;
    
    mutable u16 value;
    std::string label;
    s8 offset;
    
    Value16() = default;
    Value16(u16 value) : type(VALUE), value(value) { }
    Value16(Type type, const std::string& label, s8 offset) : type(type), label(label), offset(offset) { }
    Value16(Type type, const std::string& label) : Value16(type, label, 0) { }
  };
  
  struct Reg16
  {
    const Reg reg;
    Reg16(Reg reg) : reg(reg) { }
  };
  
  struct Reg8
  {
    const Reg reg;
    Reg8(Reg reg) : reg(reg) { }
  };
  
#pragma mark Support Instructions
  class Instruction
  {
  public:
    u8 data[4];
    u16 address;
    InstructionLength length;
    
  public:
    Instruction(InstructionLength length) : data{0}, length(length) { }
    
    virtual const u16 getLength() const { return length; }
    
    virtual bool isReal() const { return true; }
    virtual std::string mnemonic() const { return std::string(); }
    
    virtual void assemble(u8* dest) const
    {
      memcpy(dest, &data, sizeof(u8)*length);
    }
    
    virtual Result solve(const Environment& env) { return Result(); }
  };

  class Padding : public Instruction
  {
  private:
    u16 ilength;
    
  public:
    Padding(u16 ilength) : Instruction(LENGTH_0_BYTES), ilength(ilength) { }
    
    const u16 getLength() const { return ilength; }
    
    virtual std::string mnemonic() const { return fmt::sprintf("NOPx%d", ilength); }
    
    virtual void assemble(u8* dest) const
    {
      for (int i = 0; i < ilength; ++i)
        dest[i] = OPCODE_NOP;
    }
  };
  
  class InstructionXXX_NN : public Instruction
  {
  protected:
    using dest_t = u8;
    const Opcode opcode;
    const Reg8 dst;
    const Value8 value;
    
  public:
    InstructionXXX_NN(Opcode opcode, Reg8 dst, Value8 value) : Instruction(LENGTH_3_BYTES), opcode(opcode), dst(dst), value(value) { }
    
    std::string mnemonic() const override;
    Result solve(const Environment& env) override final;
  };

  
  class InstructionAddressable : public Instruction
  {
  protected:
    Address address;
    InstructionAddressable(InstructionLength length, Address address) : Instruction(length), address(address) { }
    
  public:
    
    const InterruptIndex getIntIndex() const { return address.interrupt; }
    const std::string& getLabel() const { return address.label; }
    bool mustBeSolved() const { return address.type != Address::Type::ABSOLUTE; }
    Address::Type getType() const { return address.type; }
    void solve(u16 address) { this->address.address = address; this->address.type = Address::Type::ABSOLUTE; }
  };
  
  template<typename RegType>
  class InstructionXXX_NNNN : public Instruction
  {
  protected:
    const Opcode opcode;
    const RegType dst;
    const Value16 value;
    
  public:
    InstructionXXX_NNNN(InstructionLength length, Opcode opcode, RegType dst, Value16 value) : Instruction(length), opcode(opcode), dst(dst), value(value) { }
    Result solve(const Environment& env) override final;
    std::string mnemonic() const override;
  };
  
  template<typename RegType>
  class InstructionSingleReg : public Instruction
  {
  protected:
    Opcode opcode;
    RegType reg;
    
    InstructionSingleReg(Opcode opcode, RegType reg) : Instruction(LENGTH_1_BYTES), opcode(opcode), reg(reg) { }
    std::string mnemonic() const override final;
    
  public:
    void assemble(u8* dest) const override { dest[0] = (opcode << 3) | reg.reg; }
  };
  
  template<Opcode OPCODE>
  class InstructionSimple : public Instruction
  {
  public:
    InstructionSimple() : Instruction(LENGTH_1_BYTES) { }
    std::string mnemonic() const override final { return fmt::sprintf("%s", Opcodes::opcodeName(OPCODE)); }
    void assemble(u8* dest) const override { dest[0] = OPCODE << 3; }
  };
  
  class Label : public Instruction
  {
  private:
    std::string label;
    u16 address;
    bool solved;
    
  public:
    Label(const std::string& label) : Instruction(LENGTH_0_BYTES), label(label), address(0), solved(false) { }
    
    const std::string& getLabel() const { return label; }
    
    bool isReal() const override { return false; }
    
    bool mustBeSolved() { return !solved; }
    void solve(u16 address) { this->address = address; }
  };
  
  class InterruptEntryPoint : public Instruction
  {
  private:
    u8 index;
    u16 address;
    bool solved;
    
  public:
    InterruptEntryPoint(u8 index) : Instruction(LENGTH_0_BYTES), index(index), address(0), solved(false) { }
    
    const u8 getIndex() const { return index; }
    
    bool isReal() const override { return false; }
    bool mustBeSolved() { return !solved; }
    void solve(u16 address) { this->address = address; }
  };
  
#pragma mark LD/LSH/RSH R, S, Q
  /*************
   * LD R, S
   * LSH R, S
   * RSH R, S
   *************/
  
  class InstructionXXX_R_S: public Instruction
  {
  protected:
    const Opcode opcode;
    const Reg reg1;
    const Reg reg2;
    const Alu alu;
    
  public:
    InstructionXXX_R_S(Opcode opcode, Reg reg1, Reg reg2, Alu alu, bool extended) : Instruction(LENGTH_2_BYTES),
      opcode(opcode), reg1(reg1), reg2(reg2), alu(static_cast<Alu>(alu | (extended ? 0b1 : 0)))
    { }
    
    void assemble(u8* dest) const override;    
  };
      
  class InstructionLD_LSH_RSH : public InstructionXXX_R_S
  {
  public:
    InstructionLD_LSH_RSH(Reg reg1, Reg reg2, Alu alu, bool extended) :
      InstructionXXX_R_S(OPCODE_LD_RSH_LSH, reg1, reg2, alu, extended) { }
    
    std::string mnemonic() const override;
  };
      
  class InstructionCMP_R_S : public InstructionXXX_R_S
  {
  public:
    InstructionCMP_R_S(Reg reg1, Reg reg2, bool extended) :
    InstructionXXX_R_S(OPCODE_CMP_REG, reg1, reg2, Alu::SUB8, extended) { }
    
    std::string mnemonic() const override;
  };
  
#pragma mark LD R, NN - CMP R,NN
  /*************
   * LD R, NN
   * CMP R, NN
   *************/
  class InstructionLD_NN : public InstructionXXX_NN
  {
  public:
    InstructionLD_NN(Reg dst, Value8 value) : InstructionXXX_NN(OPCODE_LD_NN, dst, value) { }
    void assemble(u8* dest) const override;
  };
  
  class InstructionCMP_NN : public InstructionXXX_NN
  {
  public:
    InstructionCMP_NN(Reg dst, Value8 value) : InstructionXXX_NN(OPCODE_CMP_NN, dst, value) { }
    void assemble(u8* dest) const override;
  };
  
  
  /*************
   * LD P, NNNN
   * ST [NNNN], R
   * LD R, [NNNN]
   * CMP P, NNNN
   * ALU P, Q, NNNN
   *************/
#pragma mark LD P, NNNN
  class InstructionLD_NNNN : public InstructionXXX_NNNN<Reg16>
  {
  public:
    InstructionLD_NNNN(Reg16 dst, Value16 value) : InstructionXXX_NNNN(LENGTH_3_BYTES, OPCODE_LD_NNNN, dst, value) { }
    void assemble(u8* dest) const override;
  };
  
#pragma mark ST [NNNN], R
  class InstructionST_PTR_NNNN : public InstructionXXX_NNNN<Reg8>
  {
  public:
    InstructionST_PTR_NNNN(Reg8 src, Value16 value) : InstructionXXX_NNNN(LENGTH_3_BYTES, OPCODE_SD_PTR_NNNN, src, value) { }
    void assemble(u8* dest) const override;
    std::string mnemonic() const override;
  };
      
#pragma mark LD R, [NNNN]
  class InstructionLD_PTR_NNNN : public InstructionXXX_NNNN<Reg8>
  {
  public:
    InstructionLD_PTR_NNNN(Reg8 src, Value16 value) : InstructionXXX_NNNN(LENGTH_3_BYTES, OPCODE_LD_PTR_NNNN, src, value) { }
    void assemble(u8* dest) const override;
    std::string mnemonic() const override;
  };
  
#pragma mark CMP P, NNNN
  class InstructionCMP_NNNN : public InstructionXXX_NNNN<Reg16>
  {
  public:
    InstructionCMP_NNNN(Reg16 dst, Value16 value) : InstructionXXX_NNNN(LENGTH_4_BYTES, OPCODE_CMP_NNNN, dst, value) { }
    void assemble(u8* dest) const override;
  };
  
#pragma mark ALU P, NNNN
  class InstructionALU_NNNN : public InstructionXXX_NNNN<Reg16>
  {
  private:
    const Reg16 src;
    const Alu alu;
  public:
    InstructionALU_NNNN(Reg16 dst, Reg16 src, Alu alu, Value16 value) : InstructionXXX_NNNN(LENGTH_4_BYTES, OPCODE_ALU_NNNN, dst, value), src(src), alu(alu) { }
    std::string mnemonic() const override;
    void assemble(u8* dest) const override;
  };
  
  /*************
   * ALU R, S, Q
   *************/
#pragma mark ALU R, S, Q
  class InstructionALU_R : public Instruction
  {
  public:
    const Alu alu;
    const Reg dst;
    const Reg src1;
    const Reg src2;
    
  public:
    InstructionALU_R(Reg dst, Reg src1, Reg src2, Alu alu, bool extended) :
    InstructionALU_R(dst, src1, src2, alu | (extended ? 0b1 : 0b0)) { }
    
    InstructionALU_R(Reg dst, Reg src1, Reg src2, Alu alu) : Instruction(LENGTH_3_BYTES),
    alu(alu), dst(dst), src1(src1), src2(src2) { }
    
    std::string mnemonic() const override;
    void assemble(byte* dest) const override;
  };
  
#pragma mark ALU R, NN
  class InstructionALU_R_NN : public InstructionXXX_NN
  {
  private:
    const Alu alu;
    const Reg8 src;
    
  public:
    InstructionALU_R_NN(Reg dst, Reg8 src, Alu alu, Value8 value) : InstructionXXX_NN(OPCODE_ALU_NN, dst, value), alu(alu), src(src) { }
    std::string mnemonic() const override;
    void assemble(byte* dest) const override;
  };
  
#pragma marg SEXT
  class InstructionSEXT : public InstructionSingleReg<Reg8>
  {
  public:
    InstructionSEXT(Reg8 reg) : InstructionSingleReg(OPCODE_SEXT, reg) { }
  };
  
#pragma mark PUSH P
  class InstructionPUSH16 : public InstructionSingleReg<Reg16>
  {
  public:
    InstructionPUSH16(Reg16 reg) : InstructionSingleReg(OPCODE_PUSH16, reg) { }
  };
  
  class InstructionPUSH8 : public InstructionSingleReg<Reg8>
  {
  public:
    InstructionPUSH8(Reg8 reg) : InstructionSingleReg(OPCODE_PUSH, reg) { }
  };
  
  
#pragma mark POP P
  class InstructionPOP8 : public InstructionSingleReg<Reg8>
  {
  public:
    InstructionPOP8(Reg8 reg) : InstructionSingleReg(OPCODE_POP, reg) { }
  };
  
  class InstructionPOP16 : public InstructionSingleReg<Reg16>
  {
  public:
    InstructionPOP16(Reg16 reg) : InstructionSingleReg(OPCODE_POP16, reg) { }
  };
      
#pragma mark LF R / SF R
  class InstructionLF : public InstructionSingleReg<Reg8>
  {
  public:
    InstructionLF(Reg8 reg) : InstructionSingleReg(OPCODE_LF, reg) { }
  };
  
  class InstructionSF : public InstructionSingleReg<Reg8>
  {
  public:
    InstructionSF(Reg8 reg) : InstructionSingleReg(OPCODE_SF, reg) { }
  };

#pragma mark JMP NNNN
  class InstructionJMP_NNNN : public InstructionAddressable
  {
  private:
    JumpCondition condition;
    
  public:
    InstructionJMP_NNNN(JumpCondition condition, Address address) : InstructionAddressable(LENGTH_3_BYTES, address), condition(condition) { }

    std::string mnemonic() const override {
      if (address.label.empty())
        return fmt::sprintf("%s%s %.4Xh", Opcodes::opcodeName(OPCODE_JMPC_NNNN), Opcodes::condName(condition), address.address);
      else
        return fmt::sprintf("%s%s %.4Xh (%s)", Opcodes::opcodeName(OPCODE_JMPC_NNNN), Opcodes::condName(condition), address.address, address.label.c_str());
    }
    
    void assemble(u8* dest) const override
    {
      dest[0] = (OPCODE_JMPC_NNNN << 3) | condition;
      dest[1] = (address.address >> 8) & 0xFF;
      dest[2] = address.address & 0xFF;
    }
  };
  
#pragma mark CALL NNNN
  class InstructionCALL_NNNN : public InstructionAddressable
  {
  private:
    JumpCondition condition;
    
  public:
    InstructionCALL_NNNN(JumpCondition condition, Address address) : InstructionAddressable(LENGTH_3_BYTES, address), condition(condition) { }
    
    std::string mnemonic() const override {
      if (address.label.empty())
        return fmt::sprintf("%s%s %.4Xh", Opcodes::opcodeName(OPCODE_CALLC), Opcodes::condName(condition), address.address);
      else
        return fmt::sprintf("%s%s %.4Xh (%s)", Opcodes::opcodeName(OPCODE_CALLC), Opcodes::condName(condition), address.address, address.label.c_str());
    }
    
    void assemble(u8* dest) const override
    {
      dest[0] = (OPCODE_CALLC << 3) | condition;
      dest[1] = (address.address >> 8) & 0xFF;
      dest[2] = address.address & 0xFF;
    }
  };
  
#pragma mark RET
  class InstructionRET : public Instruction
  {
  private:
    JumpCondition condition;
    
  public:
    InstructionRET(JumpCondition condition) : Instruction(LENGTH_1_BYTES), condition(condition) { }
    
    std::string mnemonic() const override { return fmt::sprintf("%s%s", Opcodes::opcodeName(OPCODE_RETC), Opcodes::condName(condition)); }
    void assemble(u8* dest) const override { dest[0] = (OPCODE_RETC << 3) | condition;; }
  };

#pragma mark EI / DI / NOP
  using InstructionNOP = InstructionSimple<OPCODE_NOP>;
  using InstructionEI = InstructionSimple<OPCODE_EI>;
  using InstructionDI = InstructionSimple<OPCODE_DI>;
}

#pragma Helpers for mnemonics
      
inline std::ostream& operator<<(std::ostream& os, const Assembler::Value16& value)
{
  if (value.label.empty())
    os << fmt::sprintf("%.4Xh", value.value);
  else
  {
    if (value.offset != 0)
      os << fmt::sprintf("%.4Xh (%s%+d)", value.value, value.label, value.offset);
    else
      os << fmt::sprintf("%.4Xh (%s)", value.value, value.label);
  }
  
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const Assembler::Value8& value)
{
  os << fmt::sprintf("%.2Xh", value.value);
  return os;
}

inline std::ostream& operator<<(std::ostream& os, Alu alu)
{
  os << fmt::sprintf("%s", Opcodes::aluName(alu));
  return os;
}

inline std::ostream& operator<<(std::ostream& os, Assembler::Reg16 reg)
{
  os << fmt::sprintf("%s", Opcodes::reg16(reg.reg));
  return os;
}

inline std::ostream& operator<<(std::ostream& os, Assembler::Reg8 reg)
{
  os << fmt::sprintf("%s", Opcodes::reg8(reg.reg));
  return os;
}

#endif
