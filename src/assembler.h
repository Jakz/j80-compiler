#ifndef _GB_ASSEMBLER_H_
#define _GB_ASSEMBLER_H_


#include "opcodes.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>

#include "assembler/j80lexer.h"
#include "assembler/j80parser.hpp"
#include "assembler/location.hh"

#include "format.h"

namespace Assembler
{

enum InstructionLength : u8
{
  LENGTH_1_BYTES = 1,
  LENGTH_2_BYTES = 2,
  LENGTH_3_BYTES = 3,
  LENGTH_4_BYTES = 4
};
  
#define DATA_OFFSET (0x1000)

class Instruction
{
public:
  u8 data[4];
  InstructionLength length;
  
public:
  Instruction() : data{0}, length(LENGTH_2_BYTES) { }
  Instruction(InstructionLength length) : data{0}, length(length) { }
  
  virtual const InstructionLength getLength() const { return length; }
  
  virtual std::string mnemonic() const { return std::string(); }
  
  virtual void assemble(u8* dest) const
  {
    memcpy(dest, &data, sizeof(u8)*length);
  }
};

  struct Address
  {
    enum Type
    {
      ABSOLUTE,
      LABEL
    } type;
    
    u16 address;
    std::string label;

    Address(u16 address) : type(ABSOLUTE), address(address) { }
    Address(const std::string& label) : type(LABEL), label(label) { }
  };
  
class InstructionLD_NN : public Instruction
{
private:
  Reg dst;
  u8 value;

public:
  InstructionLD_NN(Reg dst, u8 value) : dst(dst), value(value) { }
  
  std::string mnemonic() const { return fmt::sprintf("LD %s, %.2Xh", Opcodes::reg8(dst), value); }
  
  void assemble(u8* dest) const override
  {
    dest[0] = (OPCODE_LD_NN << 3) | dst;
    dest[1] = ALU_TRANSFER_B8;
    dest[2] = value;
  }
};
  
  class InstructionAddressable : public Instruction
  {
  protected:
    Address address;
    
  public:
    InstructionAddressable(InstructionLength length, Address address) : Instruction(length), address(address) { }
    
    const std::string& getLabel() { return address.label; }
    bool mustBeSolved() { return address.type == Address::Type::LABEL; }
    void solve(u16 address) { this->address.address = address; this->address.type = Address::Type::ABSOLUTE; }
  };
  
  class InstructionJMP_NNNN : public InstructionAddressable
  {
  private:
    JumpCondition condition;
    
  public:
    InstructionJMP_NNNN(JumpCondition condition, u16 address) : InstructionAddressable(LENGTH_3_BYTES, Address(address)), condition(condition) { }
    InstructionJMP_NNNN(JumpCondition condition, const std::string& label) : InstructionAddressable(LENGTH_3_BYTES, Address(label)), condition(condition) { }
    
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
    InstructionCALL_NNNN(JumpCondition condition, u16 address) : InstructionAddressable(LENGTH_3_BYTES, Address(address)), condition(condition) { }
    InstructionCALL_NNNN(JumpCondition condition, const std::string& label) : InstructionAddressable(LENGTH_3_BYTES, Address(label)), condition(condition) { }
    
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
  
  class InstructionNOP : public Instruction
  {
  public:
    InstructionNOP() : Instruction(LENGTH_1_BYTES) { }
    
    std::string mnemonic() const { return "NOP"; }
    void assemble(u8* dest) const override { dest[0] = OPCODE_NOP; }
  };
  
struct DataSegmentEntry
{
  u8 *data;
  u16 length;
  u16 offset;
  
  DataSegmentEntry() : data(nullptr), length(0), offset(0) { }
  DataSegmentEntry(const std::string& ascii) { offset = 0x0000; data = reinterpret_cast<u8*>(strdup(ascii.c_str())); length = ascii.length(); }
  DataSegmentEntry(u16 size) { offset = 0x0000; data = new u8[size](); length = size; }
  ~DataSegmentEntry() { /* TODO: can't release because it's copy constructed by STL*/ }
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

template<typename T>
struct Optional
{
private:
  T value;
  bool hasBeenSet;
  
public:
  Optional() : hasBeenSet(false) { }
  Optional(T value) : value(value), hasBeenSet(true) { }
  void set(T value) { this->value = value; hasBeenSet = true; }
  bool isSet() const { return hasBeenSet; }
  T get() const { return value; }
};

class J80Assembler
{
  private:
    u16 position;
    std::list<std::unique_ptr<Instruction>> instructions;
    std::unordered_map<std::string,u16> labels;
  
    std::vector<std::pair<u16, DataReference> > dataReferences;
  
    std::unordered_map<std::string, DataSegmentEntry> data;
  
    s8 currentIrq;
    std::list<Instruction> irqs[4];
    
    Optional<u16> entryPoint;
  
    DataSegment dataSegment;
    CodeSegment codeSegment;
  
    //static Instruction *build(InstructionLength len) { return new Instruction(len, position); }
    //static void insert(Instruction *i) { position += i->length; instructions.push_back(i); }
  
  public:
    J80Assembler();
  

  
    std::string file;
  
    void error (const Assembler::location& l, const std::string& m);
    void error (const std::string& m);
  
    bool parse(const std::string& filename);
  
    //static u16 size();
    
    bool setEntryPoint(u16 address)
    {
      if (!entryPoint.isSet())
      {
        entryPoint.set(address);
        return true;
      }
      else
        return false;
    }

    Instruction* preamble(InstructionLength len)
    {
      return new Instruction(len);
    }
  
    std::unique_ptr<Instruction>& postamble(Instruction *i)
    {
      position += i->getLength();
      instructions.push_back(std::unique_ptr<Instruction>(i));
      return instructions.back();
    }
  
    void add(Instruction* i)
    {
      position += i->getLength();
      instructions.push_back(std::unique_ptr<Instruction>(i));
    }
  
    void assembleLD_RSH_LSH(Reg dst, Reg src, AluOp opcode, bool extended)
    {
      Instruction* i = preamble(LENGTH_2_BYTES);
      if (extended) opcode = static_cast<AluOp>(opcode | 0b1);
      i->data[0] = (OPCODE_LD_RSH_LSH << 3) | dst;
      i->data[1] = (src << 5) | opcode;
      postamble(i);
    }
  
    void assembleLD_NN(Reg dst, u8 value, const std::string& label = std::string())
    {
      //TODO: add reference for length of data label
      if (!label.empty())
      {
        dataReferences.push_back(std::make_pair(position, DataReference(label, DataReference::Type::LENGTH8)));
      
        Instruction* i = preamble(LENGTH_3_BYTES);
        i->data[0] = (OPCODE_LD_NN << 3) | dst;
        i->data[1] = ALU_TRANSFER_B8;
        i->data[2] = value;
        postamble(i);
      }
    }
  
    void assembleLD_NNNN(Reg dst, u16 value, const std::string& label = std::string(), s8 offset = 0)
    {
      if (!label.empty())
        dataReferences.push_back(std::make_pair(position, DataReference(label,offset)));
      
      Instruction* i = preamble(LENGTH_3_BYTES);
      i->data[0] = (OPCODE_LD_NNNN << 3) | dst;
      i->data[2] = value & 0xFF;
      i->data[1] = (value >> 8) & 0xFF;
      postamble(i);
    }
  
    void assembleLD_PTR_NNNN(Reg dst, u16 address, const std::string& label = std::string(), s8 offset = 0)
    {
      if (!label.empty())
        dataReferences.push_back(std::make_pair(position, DataReference(label,offset)));
      
      Instruction* i = preamble(LENGTH_3_BYTES);
      i->data[0] = (OPCODE_LD_PTR_NNNN << 3) | dst;
      i->data[2] = address & 0xFF;
      i->data[1] = (address >> 8) & 0xFF;
      postamble(i);
    }
  
    void assembleLD_PTR_PP(Reg dst, Reg src, s8 value)
    {
      Instruction* i = preamble(LENGTH_3_BYTES);
      i->data[0] = (OPCODE_LD_PTR_PP << 3) | dst;
      i->data[1] = (src << 5) | ALU_ADD_NO_FLAGS;
      i->data[2] = value;
      postamble(i);
    }
  
    void assembleSD_PTR_NNNN(Reg src, u16 address, const std::string& label = std::string(), s8 offset = 0)
    {
      if (!label.empty())
        dataReferences.push_back(std::make_pair(position, DataReference(label,offset)));
      
      Instruction* i = preamble(LENGTH_3_BYTES);
      i->data[0] = (OPCODE_SD_PTR_NNNN << 3) | src;
      i->data[2] = address & 0xFF;
      i->data[1] = (address >> 8) & 0xFF;
      postamble(i);
    }
  
    void assembleSD_PTR_PP(Reg src, Reg raddr, s8 value)
    {
      Instruction* i = preamble(LENGTH_3_BYTES);
      i->data[0] = (OPCODE_SD_PTR_PP << 3) | src;
      i->data[1] = (raddr << 5) | ALU_ADD_NO_FLAGS;
      i->data[2] = value;
      postamble(i);
    }
  
    void assembleALU_REG(Reg dst, Reg src1, Reg src2, AluOp opcode, bool extended)
    {
      Instruction* i = preamble(LENGTH_3_BYTES);
      if (extended) opcode = static_cast<AluOp>(opcode | 0b1);
      
      i->data[0] = (OPCODE_ALU_REG << 3) | dst;
      i->data[1] = (src1 << 5) | opcode;
      i->data[2] = (src2 << 5);
      postamble(i);
    }
  
    void assembleALU_NN(Reg dst, Reg src1, AluOp opcode, u8 value)
    {
      Instruction* i = preamble(LENGTH_3_BYTES);
      
      i->data[0] = (OPCODE_ALU_NN << 3) | dst;
      i->data[1] = (src1 << 5) | opcode;
      i->data[2] = value;
      postamble(i);
    }
  
    void assembleALU_NNNN(Reg dst, Reg src1, AluOp opcode, u16 value)
    {
      Instruction* i = preamble(LENGTH_4_BYTES);
      opcode = static_cast<AluOp>(opcode | 0b1);
      
      i->data[0] = (OPCODE_ALU_NNNN << 3) | dst;
      i->data[1] = (src1 << 5) | opcode;
      i->data[2] = value & 0xFF;
      i->data[3] = (value >> 8) & 0xFF;
      postamble(i);
    }
  
    void assembleJMP_PP(JumpCondition cond, Reg reg)
    {
      Instruction* i = preamble(LENGTH_2_BYTES);
      
      i->data[0] = (OPCODE_JMPC_PP << 3) | cond;
      i->data[1] = (reg << 5) | ALU_TRANSFER_B16;
      
      postamble(i);
    }
    
    // CHEATED LENGTHS
    /*
     PUSH, POP, RET, LF, SF, EI, DI, INT
     
     
     */
  
    void assembleShort(Opcode opcode)
    {
      Instruction* i = preamble(LENGTH_1_BYTES);
      i->data[0] = opcode << 3;
      postamble(i);
    }
  
    void assembleShortWithReg(Opcode opcode, Reg reg)
    {
      Instruction* i = preamble(LENGTH_1_BYTES);
      i->data[0] = (opcode << 3) | reg;
      postamble(i);
    }
  
    void assembleRET(JumpCondition cond)
    {
      Instruction* i = preamble(LENGTH_1_BYTES);
      i->data[0] = (OPCODE_RETC << 3) | cond;
      postamble(i);
    }
  
    void assembleCMP_REG(Reg dst, Reg src1, bool extended)
    {
      Instruction *i = preamble(LENGTH_2_BYTES);
      AluOp opcode = ALU_SUB8;
      if (extended) opcode = ALU_SUB16;
      
      i->data[0] = (OPCODE_CMP_REG << 3) | dst;
      i->data[1] = (src1 << 5) | opcode;
      postamble(i);
    }
    
    void assembleCMP_NN(Reg dst, u8 value)
    {
      Instruction *i = preamble(LENGTH_3_BYTES);
      i->data[0] = (OPCODE_CMP_NN << 3) | dst;
      i->data[1] = ALU_SUB8;
      i->data[2] = value;
      postamble(i);
    }
    
    void assembleCMP_NNNN(Reg dst, u16 value)
    {
      Instruction *i = preamble(LENGTH_4_BYTES);
      i->data[0] = (OPCODE_CMP_NNNN << 3) | dst;
      i->data[1] = ALU_SUB16;
      i->data[2] = value & 0xFF;
      i->data[3] = (value >> 8) & 0xFF;
      postamble(i);
    }
  
  
    bool solveJumps();
  
  
    void addAsciiData(const std::string& label, const std::string& sdata)
    {
      data[label] = DataSegmentEntry(sdata);
    }
  
    void addEmptyData(const std::string& label, u16 size)
    {
      data[label] = DataSegmentEntry(size);
    }
  
    void buildDataSegment();
    void buildCodeSegment();
    void solveDataReferences();
  
    void assemble()
    {
      if (entryPoint.isSet())
        codeSegment.offset = entryPoint.get();
      
      solveJumps();
      buildCodeSegment();
      buildDataSegment();
      dataSegment.offset = codeSegment.length + codeSegment.offset;
      solveDataReferences();
    }
  
    void interruptStart(u8 index) { currentIrq = index; }
    void interruptEnd() { currentIrq = -1; }
  
    void placeLabel(const std::string& label);
  
    std::list<std::unique_ptr<Instruction>>::const_iterator iterator() { return instructions.begin(); }
    bool hasNext(std::list<std::unique_ptr<Instruction>>::const_iterator it) { return it  != instructions.end(); }

    void printProgram() const;
    void saveForLogisim(const std::string& filename) const;
    void saveBinary(const std::string& filename) const;
};
  
}

#endif