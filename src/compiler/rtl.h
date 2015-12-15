#ifndef __RTL_H__
#define __RTL_H__

#include "../utils.h"
#include "../format.h"

#include "../ast_visitor.h"

#include <string>
#include <vector>


namespace rtl
{
  enum class Comparison
  {
    EQUAL,
    NEQUAL,
    GREATER,
    GREATEREQ,
    LESS,
    LESSEQ
  };
  
  class Temporary
  {
  private:
    u32 index;
    
    static u32 counter;
    
  public:
    Temporary() : index(counter++) { }
    Temporary(u32 index) : index(index) { }
    
    const std::string getName() const { return std::string("t")+std::to_string(index); }
    
    const bool operator==(const Temporary& other) const { return other.index == index; }
    const bool operator!=(const Temporary& other) const { return !(other == *this); }
  };
  
  
  class Instruction
  {
  public:
    virtual std::string mnemonic() const = 0;
  };
  
  class Label : public Instruction
  {
  private:
    std::string label;
    
  public:
    Label(const std::string& label) : label(label) { }
    const std::string& getLabel() { return label; }
    
    std::string mnemonic() const override { return label + ":"; }
  };
  
  class Jump : public Instruction
  {
  private:
    std::string label;
    
  public:
    Jump(const std::string& label) : label(label) { }
    const std::string& getLabel() { return label; }
    
    std::string mnemonic() const override { return fmt::sprintf("JUMP(%s)", label); }
  };
  
  class ConditionalJump : public Instruction
  {
  private:
    std::string label;
    const Temporary& src1, src2;
    Comparison op;
    
  public:
    ConditionalJump(const std::string& label, const Temporary& src1, const Temporary& src2, Comparison op) : label(label), src1(src1), src2(src2), op(op) { }
    
    std::string mnemonic() const override {
      const char* sop = nullptr;
      switch (op) {
        case Comparison::EQUAL: sop = "=="; break;
        case Comparison::NEQUAL: sop = "!="; break;
        case Comparison::GREATER: sop = ">"; break;
        case Comparison::GREATEREQ: sop = ">="; break;
        case Comparison::LESS: sop = "<"; break;
        case Comparison::LESSEQ: sop = "<="; break;
      }
      
      return fmt::sprintf("CJUMP(%s %s %s, %s", src1.getName().c_str(), sop, src2.getName().c_str(), label.c_str());
    }
  };
  
  class ValueInstruction : Instruction
  {
  public:
    virtual const Temporary& getDestination() = 0;
  };
  
  enum class Operation
  {
    ADDITION
  };
  
  struct value
  {
    enum Type
    {
      BOOL_TYPE,
      BYTE_TYPE,
      WORD_TYPE,
      TEMP_TYPE,
    } type;
    
    union
    {
      bool bbvalue;
      u8 bvalue;
      u16 wvalue;
    };
    const Temporary& temp;
    
    std::string mnemonic() const
    {
      switch (type)
      {
        case BOOL_TYPE: return bbvalue ? "true" : "false";
        case BYTE_TYPE: return fmt::sprintf("%02xh", bvalue);
        case WORD_TYPE: return fmt::sprintf("%04xh", wvalue);
        case TEMP_TYPE: return temp.getName();
      }
    }
    
    value(bool b) : type(BOOL_TYPE), bbvalue(b), temp(*reinterpret_cast<const Temporary*>(NULL)) { }
    value(u8 b) : type(BYTE_TYPE), bvalue(b), temp(*reinterpret_cast<const Temporary*>(NULL)) { }
    value(u16 b) : type(WORD_TYPE), wvalue(b), temp(*reinterpret_cast<const Temporary*>(NULL)) { }
    value(const Temporary& temp) : type(TEMP_TYPE), temp(temp) { }
  };
  
  class AssignmentInstruction : public ValueInstruction
  {
  private:
    Temporary dest;
    value value;
    
  public:
    AssignmentInstruction(struct value value) : dest(), value(value) { }
    const Temporary& getDestination() override { return dest; }
    
    std::string mnemonic() const override { return fmt::sprintf("%s <- %s", dest.getName().c_str(), value.mnemonic().c_str()); }
  };
  
  class OperationInstruction : public ValueInstruction
  {
  private:
    Operation op;
    const Temporary& src1, src2;
    Temporary dest;
    
    
  public:
    OperationInstruction(const Temporary& src1, const Temporary& src2, Operation op) : src1(src1), src2(src2), dest(), op(op) { }
    
    std::string mnemonic() const override
    {
      const char* sop = nullptr;
      switch (op) {
        case Operation::ADDITION: sop = "+"; break;
      }
      
      return fmt::sprintf("%s <- %s %s %s", dest.getName().c_str(), src1.getName().c_str(), sop, src2.getName().c_str());
    }
    
    const Temporary& getDestination() override { return dest; }

  };
  
  
  class RTLBuilder : public nanoc::Visitor
  {
  private:
    std::vector<std::unique_ptr<Instruction>> code;
    
  public:
    
  };
  
  
};

#endif