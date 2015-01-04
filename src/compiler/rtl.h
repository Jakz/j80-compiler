#ifndef __RTL_H__
#define __RTL_H__

#include "../utils.h"
#include "../format.h"

#include <string>


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
    std::string name;
    static u32 counter;
    
  public:
    Temporary() : name(std::string("t") + std::to_string(counter)) { }
    Temporary(const std::string& name) : name(name) { }
    
    const std::string& getName() const { return name; }
    const char* getCName() const { return name.c_str(); }
    
    const bool operator==(const Temporary& other) const { return other.name == name; }
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
      
      return fmt::sprintf("CJUMP(%s %s %s, %s", src1.getCName(), sop, src2.getCName(), label.c_str());
    }
    
  };
  
  
};

#endif