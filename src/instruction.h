#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include <unordered_map>
#include <vector>
#include <list>

#include "format.h"
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
  
  class DataSegment
  {
  public:
    u16 offset;
    u8 *data;
    u16 length;
    
    DataSegment() : offset(0), data(nullptr), length(0) { }
    void alloc(u16 length) { if (data) delete[] data; data = new u8[length](); this->length = length;}
    ~DataSegment() { delete [] data; }
  };
  
  class CodeSegment
  {
  public:
    u16 offset;
    u8 *data;
    u16 length;
    
    CodeSegment() : offset(0), data(nullptr), length(0) { }
    void alloc(u16 length) { delete[] data; data = new u8[length](); this->length = length;}
    ~CodeSegment() { delete [] data; }
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
    
    u8 value;
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
    
    u16 value;
    std::string label;
    s8 offset;
    
    Value16() = default;
    Value16(u16 value) : type(VALUE), value(value) { }
    Value16(Type type, const std::string& label, s8 offset) : type(type), label(label), offset(offset) { }
    Value16(Type type, const std::string& label) : Value16(type, label, 0) { }
  };

  
#pragma mark LD/LSH/RSH R, S, Q
  /*************
   * LD R, S, Q
   * LSH R, S, Q
   * RSH R, S, Q
   *************/
  
  class InstructionLD_LSH_RSH : public Instruction
  {
  private:
    Reg dst;
    Reg src;
    AluOp alu;
    
  public:
    InstructionLD_LSH_RSH(Reg dst, Reg src, AluOp alu, bool extended) : Instruction(LENGTH_2_BYTES),
      dst(dst), src(src), alu(static_cast<AluOp>(alu | (extended ? 0b1 : 0)))
    { }
    
    std::string mnemonic() const override;
    
    /* 10000RRR SSSAAAAA */
    void assemble(u8* dest) const override
    {
      dest[0] = (OPCODE_LD_RSH_LSH << 3) | dst;
      dest[1] = alu | (src << 5);
    }
    
  };
  
  
#pragma mark LD R, NN - CMP R,NN
  /*************
   * LD R, NN
   * CMP R, NN
   *************/
  class InstructionXXX_NN : public Instruction
  {
  protected:
    using dest_t = u8;
    Reg dst;
    Value8 value;
    
  public:
    InstructionXXX_NN(Reg dst, Value8 value) : Instruction(LENGTH_3_BYTES), dst(dst), value(value) { }
    
    Result solve(const Environment& env) override final;
  };

  class InstructionLD_NN : public InstructionXXX_NN
  {
  public:
    InstructionLD_NN(Reg dst, Value8 value) : InstructionXXX_NN(dst, value) { }
    
    std::string mnemonic() const override;
    void assemble(u8* dest) const override;
  };
  
  class InstructionCMP_NN : public InstructionXXX_NN
  {
  public:
    InstructionCMP_NN(Reg dst, Value8 value) : InstructionXXX_NN(dst, value) { }
    
    std::string mnemonic() const override;
    void assemble(u8* dest) const override;
  };
  
  
#pragma mark LD P, NNNN
  /*************
   * LD P, NNNN
   * ST [NNNN], P
   * CMP P, NNNN
   *************/
  class InstructionXXX_NNNN : public Instruction
  {
  protected:
    Reg dst;
    Value16 value;
    
  public:
    InstructionXXX_NNNN(InstructionLength length, Reg dst, Value16 value) : Instruction(length), dst(dst), value(value) { }
    Result solve(const Environment& env) override final;
  };
  
  class InstructionLD_NNNN : public InstructionXXX_NNNN
  {
  public:
    InstructionLD_NNNN(Reg dst, Value16 value) : InstructionXXX_NNNN(LENGTH_3_BYTES, dst, value) { }
    
    std::string mnemonic() const override;
    void assemble(u8* dest) const override;
  };
  
  class InstructionST_NNNN : public InstructionXXX_NNNN
  {
  public:
    InstructionST_NNNN(Reg src, Value16 value) : InstructionXXX_NNNN(LENGTH_3_BYTES, src, value) { }
    
    std::string mnemonic() const override;
    void assemble(u8* dest) const override;
  };
  
  class InstructionCMP_NNNN : public InstructionXXX_NNNN
  {
  public:
    InstructionCMP_NNNN(Reg dst, Value16 value) : InstructionXXX_NNNN(LENGTH_4_BYTES, dst, value) { }
    
    std::string mnemonic() const override;
    void assemble(u8* dest) const override;
  };
  
  
#pragma mark ALU R, S, Q
  /*************
   * ALU R, S, Q
   *************/
  class InstructionALU_R : public Instruction
  {
  public:
    AluOp alu;
    Reg dst;
    Reg src1;
    Reg src2;
    
  public:
    InstructionALU_R(Reg dst, Reg src1, Reg src2, AluOp alu, bool extended) :
    InstructionALU_R(dst, src1, src2, alu | (extended ? 0b1 : 0b0)) { }
    
    InstructionALU_R(Reg dst, Reg src1, Reg src2, AluOp alu) : Instruction(LENGTH_3_BYTES),
    alu(alu), dst(dst), src1(src1), src2(src2) { }
    
    std::string mnemonic() const override;
    void assemble(byte* dest) const override;
  };
  
  class InstructionSingleReg : public Instruction
  {
  protected:
    Opcode opcode;
    Reg reg;
    
    InstructionSingleReg(Opcode opcode, Reg reg) : Instruction(LENGTH_1_BYTES), opcode(opcode), reg(reg) { }
    
  public:
    void assemble(u8* dest) const override { dest[0] = (opcode << 3) | reg; }
  };
  
  class InstructionSEXT : public InstructionSingleReg
  {
  private:
    
  public:
    InstructionSEXT(Reg reg) : InstructionSingleReg(OPCODE_SEXT, reg) { }
    std::string mnemonic() const { return fmt::sprintf("SEXT %s", Opcodes::reg8(reg)); }
  };
  
#pragma mark PUSH P
  class InstructionPUSH16 : public InstructionSingleReg
  {
  private:
    
  public:
    InstructionPUSH16(Reg reg) : InstructionSingleReg(OPCODE_PUSH16, reg) { }
    std::string mnemonic() const { return fmt::sprintf("PUSH %s", Opcodes::reg16(reg)); }
  };
  
#pragma mark POP P
  class InstructionPOP16 : public InstructionSingleReg
  {
  private:
    
  public:
    InstructionPOP16(Reg reg) : InstructionSingleReg(OPCODE_POP16, reg) { }
    std::string mnemonic() const { return fmt::sprintf("POP %s", Opcodes::reg16(reg)); }
  };
  
#pragma mark InstructionAddressable
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
  
  class InstructionJMP_NNNN : public InstructionAddressable
  {
  private:
    JumpCondition condition;
    
  public:
    InstructionJMP_NNNN(JumpCondition condition, Address address) : InstructionAddressable(LENGTH_3_BYTES, address), condition(condition) { }

    std::string mnemonic() const override {
      if (address.label.empty())
        return fmt::sprintf("JMP%s %.4Xh", Opcodes::condName(condition), address.address);
      else
        return fmt::sprintf("JMP%s %.4Xh (%s)", Opcodes::condName(condition), address.address, address.label.c_str());
    }
    
    void assemble(u8* dest) const override
    {
      dest[0] = (OPCODE_JMPC_NNNN << 3) | condition;
      dest[1] = (address.address >> 8) & 0xFF;
      dest[2] = address.address & 0xFF;
    }
  };
  
  class InstructionCALL_NNNN : public InstructionAddressable
  {
  private:
    JumpCondition condition;
    
  public:
    InstructionCALL_NNNN(JumpCondition condition, Address address) : InstructionAddressable(LENGTH_3_BYTES, address), condition(condition) { }
    
    std::string mnemonic() const override {
      if (address.label.empty())
        return fmt::sprintf("CALL%s %.4Xh", Opcodes::condName(condition), address.address);
      else
        return fmt::sprintf("CALL%s %.4Xh (%s)", Opcodes::condName(condition), address.address, address.label.c_str());
    }
    
    void assemble(u8* dest) const override
    {
      dest[0] = (OPCODE_CALLC << 3) | condition;
      dest[1] = (address.address >> 8) & 0xFF;
      dest[2] = address.address & 0xFF;
    }
  };
  
  class InstructionRET : public Instruction
  {
  private:
    JumpCondition condition;
    
  public:
    InstructionRET(JumpCondition condition) : Instruction(LENGTH_1_BYTES), condition(condition) { }
    
    std::string mnemonic() const override { return fmt::sprintf("RET%s", Opcodes::condName(condition)); }
    void assemble(u8* dest) const override { dest[0] = (OPCODE_RETC << 3) | condition;; }
  };
  
  class InstructionNOP : public Instruction
  {
  public:
    InstructionNOP() : Instruction(LENGTH_1_BYTES) { }
    
    std::string mnemonic() const override { return "NOP"; }
    void assemble(u8* dest) const override { dest[0] = OPCODE_NOP; }
  };
}

#endif
