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
    bool constant;
    
    static u32 counter;
    
  public:
    explicit Temporary() : index(counter++), constant(false) { }
    Temporary(u32 index) : index(index), constant(false) { }
    
    const std::string getName() const { return std::string("t")+std::to_string(index); }
    
    const bool operator==(const Temporary& other) const { return other.index == index; }
    const bool operator!=(const Temporary& other) const { return !(other == *this); }
    
    void makeConstant() { constant = true; }

    bool isValid() const { return index != -1; }
    bool isConstant() const { return constant; }
  };

  enum class Branch { None, After, Before };
  
  class Instruction
  {
  public:
    virtual ~Instruction() { }
    virtual std::string mnemonic() const = 0;
    virtual Branch branch() const { return Branch::None; }
  };
  
  class Label : public Instruction
  {
  private:
    std::string _label;
    
  public:
    Label(const std::string& label) : _label(label) { }
    const std::string& label() const { return _label; }
    
    std::string mnemonic() const override { return _label + ":"; }

    Branch branch() const override { return Branch::Before; }
  };
  
  class Jump : public Instruction
  {
  protected:
    std::string _label;
    
  public:
    Jump(const std::string& label) : _label(label) { }
    
    const std::string& label() const { return _label; }
    std::string mnemonic() const override { return fmt::format("JUMP({})", _label); }

    Branch branch() const override { return Branch::After; }
  };
  
  class ConditionalJump : public Jump
  {
  private:
    const Temporary& condition;
    bool negate;
    
  public:
    ConditionalJump(const std::string& label, const Temporary& condition, bool negate) : Jump(label), condition(condition), negate(negate) { }
    
    std::string mnemonic() const override {      
      return fmt::format("CJUMP({}, {}{})", _label, negate ? "!" : "", condition.getName());
    }

    Branch branch() const override { return Branch::After; }
  };
  
  class ValueInstruction : public Instruction
  {
  public:
    virtual const Temporary& getDestination() = 0;
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
        case BYTE_TYPE: return fmt::format("{:02x}h", bvalue);
        case WORD_TYPE: return fmt::format("{}", wvalue);
        case TEMP_TYPE: return temp.getName();
        case REF_TYPE: return rtype;
      }
    }
    
    value(bool b) : type(BOOL_TYPE), bbvalue(b), temp(*reinterpret_cast<const Temporary*>(NULL)) { }
    //value(u8 b) : type(BYTE_TYPE), bvalue(b), temp(*reinterpret_cast<const Temporary*>(NULL)) { }
    value(u16 b) : type(WORD_TYPE), wvalue(b), temp(*reinterpret_cast<const Temporary*>(NULL)) { }
    value(const std::string& rtype) : type(REF_TYPE), rtype(rtype), temp(*reinterpret_cast<const Temporary*>(NULL)) { }
    value(const Temporary& temp) : type(TEMP_TYPE), temp(temp) { }

    bool isConstant() const { return type != REF_TYPE && (type != TEMP_TYPE || temp.isConstant()); }
  };
  
  class AssignmentInstruction : public ValueInstruction
  {
  private:
    Temporary dest;
    value value;
    
  public:
    AssignmentInstruction(struct value value) : dest(), value(value) { }
    const Temporary& getDestination() override { return dest; }
    
    std::string mnemonic() const override { return fmt::format("ASSIGN({}, {})", dest.getName(), value.mnemonic()); }
  };
  
  class OperationInstruction : public ValueInstruction
  {
  private:
    nanoc::Binary op;
    const Temporary& src1, src2;
    Temporary dest;
    
    
  public:
    OperationInstruction(const Temporary& src1, const Temporary& src2, nanoc::Binary op) : src1(src1), src2(src2), dest(), op(op) { }
    
    std::string mnemonic() const override
    {
      const char* sop = "=|=";
      switch (op)
      {
        case nanoc::Binary::ADDITION: sop = "+"; break;
        case nanoc::Binary::SUBTRACTION: sop = "-"; break;

        case nanoc::Binary::EQ: sop = "=="; break;
        case nanoc::Binary::NEQ: sop = "!="; break;
        case nanoc::Binary::GREATER: sop = ">"; break;
        case nanoc::Binary::GREATEREQ: sop = ">="; break;
        case nanoc::Binary::LESS: sop = "<"; break;
        case nanoc::Binary::LESSEQ: sop = "<="; break;

        case nanoc::Binary::LOR: sop = "||"; break;
      }
      
      return fmt::format("EVAL({}, {} {} {})", dest.getName(), src1.getName(), sop, src2.getName());
    }
    
    const Temporary& getDestination() override { return dest; }
  };

  class Return : public Instruction
  {
  private:
    const Temporary& value;

  public:
    Return(const Temporary& value) : value(value) { }

    std::string mnemonic() const override { return fmt::format("RETN({})", value.getName()); }
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
      std::string r = fmt::format("CALL({}", function);
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

  struct InstructionBlock
  {
    size_t index;
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::vector<InstructionBlock*> outgoing;

    InstructionBlock();

    void add(Instruction* i) { instructions.push_back(std::unique_ptr<Instruction>(i)); }

    auto begin() { return instructions.begin(); }
    auto end() { return instructions.end(); }
    auto begin() const { return instructions.begin(); }
    auto end() const { return instructions.end(); }

    void link(InstructionBlock* block) { outgoing.push_back(block); }

    const Instruction* first() const { return instructions.front().get(); }
    const Instruction* last() const { return instructions.back().get(); }

  };
  
  class Procedure
  {
  public:
    std::string name;
    std::vector<Argument> arguments;
    bool hasReturnValue;
    
    std::vector<std::unique_ptr<InstructionBlock>> instructions;
    
    std::string mnemonic();

    Procedure() : hasReturnValue(false)
    {
      addBlock();
    }

    InstructionBlock* addBlock()
    {
      instructions.push_back(std::make_unique<InstructionBlock>());
      return instructions.back().get();
    }

    void buildCFG();
  };
  
  
  
  class RTLBuilder : public nanoc::Visitor
  {
  private:
    std::vector<std::unique_ptr<Procedure>> code;
    std::stack<std::reference_wrapper<const Temporary>> temporaries;
    Procedure* currentProcedure;

    s32 ifLabelCounter;

    void add(Instruction* i) { currentProcedure->instructions[0]->add(i); }
    std::string label(const std::string& type, s32 v) const { return fmt::format("{}{}", type, v); }
    
  public:
    RTLBuilder() : currentProcedure(nullptr), ifLabelCounter(0) { }
    
    nanoc::ASTNode* exitingNode(nanoc::ASTReference* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTNumber* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTBinaryExpression* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTCall* node) override;

    void stepNode(nanoc::ASTIfBlock* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTIfBlock* node) override;

    nanoc::ASTNode* exitingNode(nanoc::ASTReturn* node) override;

    void enteringNode(nanoc::ASTFuncDeclaration* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTFuncDeclaration* node) override;

    void computeConstants();
    
    void print();
  };
  
  
};

#endif
