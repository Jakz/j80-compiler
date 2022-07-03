#ifndef __RTL_H__
#define __RTL_H__

#include "utils.h"
#include "support/format/format.h"

#include "ast_visitor.h"

#include <string>
#include <vector>
#include <stack>

namespace nanoc
{
  class ASTFuncDeclaration;
  class ASTBinaryExpression;
  class ASTNode;
  class ASTCall;
}

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
    s32 index;
    
    static u32 counter;
    
  public:
    explicit Temporary() : index(counter++) { }
    Temporary(u32 index) : index(index) { }
    
    const std::string getName() const { return std::string("t")+std::to_string(index); }
    
    const bool operator==(const Temporary& other) const { return other.index == index; }
    const bool operator!=(const Temporary& other) const { return !(other == *this); }
    
    bool isValid() const { return index != -1; }
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
    
    std::string mnemonic() const override { return fmt::format("JUMP(%s)", label); }
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
      
      return fmt::format("CJUMP(%s %s %s, %s", src1.getName().c_str(), sop, src2.getName().c_str(), label.c_str());
    }
  };
  
  class ValueInstruction : public Instruction
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
      REF_TYPE,
    } type;
    
    union
    {
      bool bbvalue;
      u8 bvalue;
      u16 wvalue;
    };
    std::string rtype;
    const Temporary& temp;
    
    std::string mnemonic() const
    {
      switch (type)
      {
        case BOOL_TYPE: return bbvalue ? "true" : "false";
        case BYTE_TYPE: return fmt::format("%02xh", bvalue);
        case WORD_TYPE: return fmt::format("%u", wvalue);
        case TEMP_TYPE: return temp.getName();
        case REF_TYPE: return rtype;
      }
    }
    
    value(bool b) : type(BOOL_TYPE), bbvalue(b), temp(*reinterpret_cast<const Temporary*>(NULL)) { }
    //value(u8 b) : type(BYTE_TYPE), bvalue(b), temp(*reinterpret_cast<const Temporary*>(NULL)) { }
    value(u16 b) : type(WORD_TYPE), wvalue(b), temp(*reinterpret_cast<const Temporary*>(NULL)) { }
    value(const std::string& rtype) : type(REF_TYPE), rtype(rtype), temp(*reinterpret_cast<const Temporary*>(NULL)) { }
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
    
    std::string mnemonic() const override { return fmt::format("ASSIGN(%s, %s)", dest.getName().c_str(), value.mnemonic().c_str()); }
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
      
      return fmt::format("EVAL(%s, %s %s %s)", dest.getName().c_str(), src1.getName().c_str(), sop, src2.getName().c_str());
    }
    
    const Temporary& getDestination() override { return dest; }
  };
  
  class CallInstruction : public Instruction
  {
  private:
    std::string function;
    std::vector<std::reference_wrapper<const Temporary>> arguments;
    Temporary returnValue;
  public:
    CallInstruction(const std::string& name, const decltype(arguments)& arguments) : function(name), arguments(arguments), returnValue(-1) { }
    CallInstruction(const std::string& name, const decltype(arguments)& arguments, bool hasReturnValue) : function(name), arguments(arguments), returnValue() { }
    
    std::string mnemonic() const override
    {
      std::string r = fmt::format("CALL(%s", function);
      if (arguments.empty() && !returnValue.isValid())
        return r+")";
      else
      {
        r += ", ";
        if (!arguments.empty())
        {
          for (int i = 0; i < arguments.size(); ++i)
          {
            r += arguments[i].get().getName();
            if (i < arguments.size()-1)
              r += ", ";
          }
        }
        
        if (returnValue.isValid())
        {
          if (!arguments.empty()) r+= ", "; else r+=" ";
          r += "ret: " + returnValue.getName();
        }
        
        return r+")";
      }
    }
  };
  
  
  struct Argument
  {
    std::string name;
    Temporary temporary;
  };
  
  class Procedure
  {
  public:
    std::string name;
    std::vector<Argument> arguments;
    bool hasReturnValue;
    
    std::vector<std::unique_ptr<Instruction>> instructions;
    
    std::string mnemonic();
  };
  
  
  
  class RTLBuilder : public nanoc::Visitor
  {
  private:
    std::vector<std::unique_ptr<Procedure>> code;
    std::stack<std::reference_wrapper<const Temporary>> temporaries;
    Procedure* currentProcedure;
    
  public:
    RTLBuilder() : currentProcedure(nullptr) { }
    
    nanoc::ASTNode* exitingNode(nanoc::ASTReference* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTNumber* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTBinaryExpression* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTCall* node) override;

    void enteringNode(nanoc::ASTFuncDeclaration* node) override;
    
    void print();
  };
  
  
};

#endif
