#ifndef __RTL_H__
#define __RTL_H__

#include "utils.h"
#include "support/format/format.h"

#include "ast_visitor.h"

#include <string>
#include <vector>
#include <set>
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
    
    Temporary(u32 index) : index(index), constant(false) { }

  public:
    Temporary() : index(-1) { }
    
    const std::string getName() const { return std::string("t")+std::to_string(index); }
    s32 i() const { return index; }
    
    const bool operator<(const Temporary& other) const { return other.index < index; }
    const bool operator==(const Temporary& other) const { return other.index == index; }
    const bool operator!=(const Temporary& other) const { return !(other == *this); }
    
    void makeConstant() { constant = true; }

    bool isValid() const { return index != -1; }
    bool isConstant() const { return constant; }

    static Temporary generate() { return Temporary(counter++); }
    static Temporary invalid() { return Temporary(-1); }
  };

  struct value
  {
    enum class Type { Bool, Byte, Word, Temp, Ref };
    Type type;

    union
    {
      bool bbvalue;
      u8 bvalue;
      u16 wvalue;
    };
    std::string rtype;
    Temporary temp;

    std::string mnemonic() const
    {
      switch (type)
      {
        case Type::Bool: return bbvalue ? "true" : "false";
        case Type::Byte: return fmt::format("{:02x}h", bvalue);
        case Type::Word: return fmt::format("{}", wvalue);
        case Type::Temp: return temp.getName();
        case Type::Ref: return rtype;
      }
    }

    value(bool b) : type(Type::Bool), bbvalue(b), temp(Temporary::invalid()) { }
    //value(u8 b) : type(BYTE_TYPE), bvalue(b), temp(Temporary(-1)) { }
    value(u16 b) : type(Type::Word), wvalue(b), temp(Temporary::invalid()) { }
    value(const std::string& rtype) : type(Type::Ref), rtype(rtype), temp(Temporary::invalid()) { }
    value(const Temporary& temp) : type(Type::Temp), temp(temp) { }

    bool isConstant() const { return type != Type::Ref && (type != Type::Temp || temp.isConstant()); }

    const bool operator==(const Temporary& other) const { return type == Type::Ref && other.i() == temp.i(); }
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
    value _condition;
    bool _negate;
    
  public:
    ConditionalJump(const std::string& label, const value& condition, bool negate) : Jump(label), _condition(condition), _negate(negate) { }
    
    std::string mnemonic() const override {      
      return fmt::format("CJUMP({}, {}{})", _label, _negate ? "!" : "", _condition.mnemonic());
    }

    Branch branch() const override { return Branch::After; }
  };
  
  class ValueInstruction : public Instruction
  {
  public:
    virtual const Temporary& destination() const = 0;
  };
    
  class Assignment : public ValueInstruction
  {
  private:
    Temporary _dest;
    value _value;
    
  public:
    Assignment(const Temporary& dest, struct value value) : _dest(dest), _value(value) { }
    const Temporary& destination() const override { return _dest; }
    const value& value() const  { return _value; }
    
    std::string mnemonic() const override { return fmt::format("ASSIGN({}, {})", _dest.getName(), _value.mnemonic()); }
  };
  
  class OperationInstruction : public ValueInstruction
  {
  private:
    nanoc::Binary _op;
    value _src1, _src2;
    Temporary _dest;
    
    
  public:
    OperationInstruction(const value& src1, const value& src2, nanoc::Binary op) : _src1(src1), _src2(src2), _dest(Temporary::generate()), _op(op) { }
    
    std::string mnemonic() const override
    {
      const char* sop = "=|=";
      switch (_op)
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
      
      return fmt::format("EVAL({}, {} {} {})", _dest.getName(), _src1.mnemonic(), sop, _src2.mnemonic());
    }
    
    const Temporary& destination() const override { return _dest; }
    const value& operand1() const { return _src1; }
    const value& operand2() const { return _src2; }
  };

  class Return : public Instruction
  {
  private:
    value _value;

  public:
    Return(const value& value) : _value(value) { }

    std::string mnemonic() const override { return fmt::format("RETN({})", _value.mnemonic()); }
    const auto& value() const { return _value; }
  };
  
  class CallInstruction : public Instruction
  {
  private:
    std::string function;
    std::vector<value> arguments;
    Temporary returnValue;
  public:
    CallInstruction(const std::string& name, const decltype(arguments)& arguments) : function(name), arguments(arguments), returnValue(Temporary::invalid()) { }
    CallInstruction(const std::string& name, const decltype(arguments)& arguments, bool hasReturnValue) : function(name), arguments(arguments), returnValue(hasReturnValue ? Temporary::generate() : Temporary::invalid()) { }
    
    std::string mnemonic() const override;

    const auto& args() const { return arguments; }
    const auto& retnValue() const { return returnValue; }
  };
  
  struct Argument
  {
    std::string name;
    Temporary temporary;
  };

  struct LiveSet
  {
    std::set<Temporary> data;

    void add(const Temporary& temporary) { data.insert(temporary); }
    bool contains(const Temporary& temp) const
    {
      return data.find(temp) != data.end();
    }

    LiveSet operator+(const LiveSet& other) const
    {
      LiveSet r;
      r.data.insert(data.begin(), data.end());
      r.data.insert(other.data.begin(), other.data.end());
      return r;
    }

    LiveSet operator-(const LiveSet& other) const
    {
      LiveSet r;
      std::set_difference(data.begin(), data.end(), other.data.begin(), other.data.end(),
        std::inserter(r.data, r.data.end()));
      return r;
    }

    LiveSet& operator+=(const LiveSet& other)
    {
      data.insert(other.data.begin(), other.data.end());
      return *this;
    }

    bool operator==(const LiveSet& other) const { return data == other.data; }
    bool operator!=(const LiveSet& other) const { return data != other.data; }

    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
  };

  struct LiveData
  {
    LiveSet in, out, def, use;
  };

  struct InstructionBlock
  {
    size_t index;
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::vector<InstructionBlock*> outgoing;

    LiveData live;

    InstructionBlock();

    void add(Instruction* i) { instructions.push_back(std::unique_ptr<Instruction>(i)); }

    bool empty() const { return instructions.empty(); }
    auto begin() { return instructions.begin(); }
    auto end() { return instructions.end(); }
    auto begin() const { return instructions.begin(); }
    auto end() const { return instructions.end(); }

    void link(InstructionBlock* block) { outgoing.push_back(block); }

    void setIndex(size_t index) { this->index = index; }

    const Instruction* first() const { return instructions.front().get(); }
    const Instruction* last() const { return instructions.back().get(); }
  };
  
  class Procedure
  {
  public:
    std::string name;
    std::vector<Argument> arguments;
    std::unordered_map<std::string, Temporary> locals;
    bool hasReturnValue;
    
    std::vector<std::unique_ptr<InstructionBlock>> blocks;
    
    std::string mnemonic();

    Procedure() : hasReturnValue(false)
    {
      addBlock();
    }

    InstructionBlock* addBlock()
    {
      blocks.push_back(std::make_unique<InstructionBlock>());
      return blocks.back().get();
    }

    void buildCFG();
    void liveAnalysis();
  };
  
  
  
  class RTLBuilder : public nanoc::Visitor
  {
  private:
    std::vector<std::unique_ptr<Procedure>> code;
    std::stack<value> values;
    Procedure* currentProcedure;

    s32 ifLabelCounter;
    s32 whileLabelCounter;

    void add(Instruction* i) { currentProcedure->blocks[0]->add(i); }
    std::string label(const std::string& type, s32 v) const { return fmt::format("{}{}", type, v); }
    
  public:
    RTLBuilder() : currentProcedure(nullptr), ifLabelCounter(0), whileLabelCounter(0) { }
    
    nanoc::ASTNode* exitingNode(nanoc::ASTReference* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTNumber* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTBinaryExpression* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTCall* node) override;

    void enteringNode(nanoc::ASTDeclarationValue* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTDeclarationValue* node) override;

    void stepNode(nanoc::ASTIfBlock* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTIfBlock* node) override;

    void enteringNode(nanoc::ASTWhile* node) override;
    void stepNode(nanoc::ASTWhile* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTWhile* node) override;

    nanoc::ASTNode* exitingNode(nanoc::ASTReturn* node) override;

    void enteringNode(nanoc::ASTFuncDeclaration* node) override;
    nanoc::ASTNode* exitingNode(nanoc::ASTFuncDeclaration* node) override;

    void computeConstants();
    
    void print();
  };
  
  
};

#endif
