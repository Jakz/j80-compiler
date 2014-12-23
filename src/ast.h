#ifndef __AST_H__
#define __AST_H__

#include <memory>
#include <vector>
#include <list>
#include <string>

#include "format.h"
#include "utils.h"
#include "ast_visitor.h"

namespace nanoc
{
  enum Type
  {
    VOID,
    BOOL,
    BYTE,
    WORD,
    BOOL_PTR,
    BYTE_PTR,
    WORD_PTR,
    BOOL_ARRAY,
    BYTE_ARRAY,
    WORD_ARRAY,
  };

  enum Unary
  {
    NOT,
    INCR,
    DECR,
    NEG
  };
  
  enum Binary
  {
    ADDITION,
    SUBTRACTION,
    AND,
    OR,
    XOR,
    
    EQ,
    NEQ,
    GREATEREQ,
    LESSEQ,
    GREATER,
    LESS,
    
  };
  
  typedef signed int Value;
  
  class Mnemonics
  {
  public:
    static std::string mnemonicForUnary(Unary op);
    static std::string mnemonicForBinary(Binary op);
    static std::string mnemonicForType(Type type, u16 param = 0);
  };
  
  
  struct Argument
  {
    std::string name;
    Type type;
    
    Argument(std::string name, Type type) : name(name), type(type) { }
  };

  class ASTNode;
  using UniqueNode = std::unique_ptr<ASTNode>;
  class ASTExpression;
  using UniqueExpression = std::unique_ptr<ASTExpression>;
  template<typename T>
  class ASTList;
  template<typename T>
  using UniqueList = std::unique_ptr<ASTList<T>>;

  class ASTNode
  {
    virtual void ivisit(Visitor* visitor) { }
    
  public:
    void visit(Visitor* visitor)
    {
      visitor->visit(this);
      visitor->enteringNode(this);
      ivisit(visitor);
      visitor->exitingNode(this);
    }
    
    virtual std::string mnemonic() const = 0;
  };
  
  template<typename T>
  class ASTList : public ASTNode
  {
  private:
    std::list<std::unique_ptr<T>> statements;
    
    std::string mnemonic() const override {
      std::string unmangledName = Utils::execute(std::string("c++filt ")+typeid(T).name());
      unmangledName.erase(unmangledName.end()-1);
      size_t namespaceIndex = unmangledName.find("::");
      unmangledName = unmangledName.substr(namespaceIndex+2);
      return fmt::sprintf("List<%s>", unmangledName.c_str());
    }
    
  public:
    ASTList() { }
    ASTList(std::list<T*>& statements)
    {
      for (auto* s : statements)
        this->statements.push_back(std::unique_ptr<T>(s));
    }
    
    void ivisit(Visitor* v) { for (const auto& c : statements) c->visit(v); }
    
  };

  class ASTStatement : public ASTNode { };
  class ASTDeclaration : public ASTNode { };
  
  
  
  class ASTExpression : public ASTStatement
  {
  public:
    // TODO: make pure virtual and implement in subtypes
    virtual Type getType() const { return Type::WORD; }
  };
  
  class ASTNumber : public ASTExpression
  {
  private:
    Value value;
    
  public:
    ASTNumber(Value value) : value(value) { }
    std::string mnemonic() const override { return fmt::sprintf("Number(%d)", value); }
  };
  
  class ASTBool : public ASTExpression
  {
  private:
    bool value;
    
  public:
    ASTBool(bool value) : value(value) { }
    std::string mnemonic() const override { return value ? "true" : "false"; }
  };
  
  class ASTReference : public ASTExpression
  {
  private:
    std::string name;
    
  public:
    ASTReference(const std::string& name) : name(name) { }
    std::string mnemonic() const override { return fmt::sprintf("Reference(%s)", name.c_str()); }
  };
  
  class ASTCall : public ASTExpression
  {
  private:
    std::string name;
    UniqueList<ASTExpression> arguments;
    
    std::string mnemonic() const override { return fmt::sprintf("Call(%s)", name.c_str()); }
    
  public:
    ASTCall(const std::string& name) : name(name) { }
    ASTCall(const std::string& name, std::list<ASTExpression*>& arguments) : name(name),
      arguments(UniqueList<ASTExpression>(new ASTList<ASTExpression>(arguments))) { }
    
    void ivisit(Visitor* visitor) override { arguments->visit(visitor); }
  };
  
  
  class ASTBinaryExpression : public ASTExpression
  {
  private:
    Binary op;
    UniqueExpression operand1, operand2;
    
    std::string mnemonic() const override { return fmt::sprintf("BinaryExpression(%s)", Mnemonics::mnemonicForBinary(op)); }
    
  public:
    ASTBinaryExpression(Binary op, ASTExpression* operand1, ASTExpression* operand2) : op(op), operand1(UniqueExpression(operand1)), operand2(UniqueExpression(operand2)) { }
    
    void ivisit(Visitor* visitor) override { operand1->visit(visitor); operand2->visit(visitor); }
  };
  
  
  class ASTUnaryExpression : public ASTExpression
  {
  private:
    Unary op;
    UniqueExpression operand;
    
    std::string mnemonic() const override { return fmt::sprintf("UnaryExpression(%s)", Mnemonics::mnemonicForUnary(op)); }
    
  public:
    ASTUnaryExpression(Unary op, ASTExpression* operand) : op(op), operand(UniqueExpression(operand)) { }
    
    void ivisit(Visitor* visitor) override { operand->visit(visitor); }
  };

  class ASTLeftHand : public ASTNode
  {
  private:
    std::string name;
    
  public:
    ASTLeftHand(const std::string& name) : name(name) { }
    
    std::string mnemonic() const override { return fmt::sprintf("%s", name.c_str()); }
  };

  class ASTAssign : public ASTStatement
  {
  private:
    std::unique_ptr<ASTLeftHand> leftHand;
    UniqueExpression expression;
    
  public:
    ASTAssign(ASTLeftHand *leftHand, ASTExpression* expression) : leftHand(std::unique_ptr<ASTLeftHand>(leftHand)), expression(UniqueExpression(expression))
    {
      
    }
    
    std::string mnemonic() const override { return fmt::sprintf("Assign(%s)", leftHand->mnemonic().c_str()); }
    void ivisit(Visitor* visitor) override { expression->visit(visitor); }
  };


  class ASTVariableDeclaration : public ASTDeclaration
  {
  private:
    std::string name;
    
  protected:
    ASTVariableDeclaration(const std::string& name) : name(name) { }
    
    std::string mnemonic() const override { return fmt::sprintf("Declaration(%s, %s)", name.c_str(), getTypeName().c_str()); }
    
  public:
    virtual Type getType() const = 0;
    virtual std::string getTypeName() const = 0;
  };
  
  template<Type T>
  class ASTDeclarationValue : public ASTVariableDeclaration
  {
  private:
    UniqueExpression value; // TODO: if a signed value is stored here print will be incorrect
  public:
    ASTDeclarationValue(const std::string& name, ASTExpression* value = nullptr) : ASTVariableDeclaration(name), value(UniqueExpression(value)) { }
    Type getType() const override { return T; }
    std::string getTypeName() const override  { return Mnemonics::mnemonicForType(T); }
    
    void ivisit(Visitor* visitor) override { value->visit(visitor); }
  };

  class ASTDeclarationPtr : public ASTVariableDeclaration
  {
  private:
    u16 address;
    Type type;
  public:
    ASTDeclarationPtr(const std::string& name, Type type, u16 address = 0) : ASTVariableDeclaration(name), type(type), address(address) { }
    Type getType() const override { return type; }
    Type getItemType() const { return type == Type::WORD_PTR ? Type::WORD : Type::BYTE; }
    std::string getTypeName() const override  { return type == Type::WORD_PTR ? "word*" : "byte*"; }

  };

  class ASTDeclarationArray : public ASTVariableDeclaration
  {
  private:
    u16 length;
    Type type;
  public:
    ASTDeclarationArray(const std::string& name, Type type, u16 length = 0) : ASTVariableDeclaration(name), type(type), length(length) { }
    Type getType() const override { return type; }
    Type getItemType() const { return type == Type::WORD_PTR ? Type::WORD : Type::BYTE; }
    std::string getTypeName() const override  { return  std::string(type == Type::WORD_PTR ? "word[" : "byte[")+std::to_string(length)+"]"; }
  };

  class ASTFuncDeclaration : public ASTDeclaration
  {
    std::string name;
    Type returnType;
    std::list<Argument> arguments;
    UniqueList<ASTStatement> statements;
    
    std::string mnemonic() const override
    {
      std::string mnemonic = fmt::sprintf("FunctionDeclaration(%s, %s", name.c_str(), Mnemonics::mnemonicForType(returnType));

      if (!arguments.empty())
      {
        mnemonic += ", [";
        bool first = true;
        for (const auto& arg : arguments)
        {
          if (!first) mnemonic += ", ";
          mnemonic += fmt::sprintf("%s %s", Mnemonics::mnemonicForType(arg.type), arg.name.c_str());
          first = false;
        }
        mnemonic += "]";
      }
      mnemonic += ")";
      
      return mnemonic;
    }
    
  public:
    ASTFuncDeclaration(std::string name, Type returnType, std::list<Argument>& arguments, std::list<ASTStatement*>& statements) : name(name), returnType(returnType),
    arguments(arguments), statements(UniqueList<ASTStatement>(new ASTList<ASTStatement>(statements))) { }

    void ivisit(Visitor* visitor) override { statements->visit(visitor); }

  };


class ASTWhile : public ASTStatement
{
private:
  UniqueExpression condition;
  UniqueList<ASTStatement> statements;
  
  std::string mnemonic() const override { return "While"; }
  
public:
  ASTWhile(ASTExpression* condition, std::list<ASTStatement*> statements) : condition(UniqueExpression(condition)),
    statements(UniqueList<ASTStatement>(new ASTList<ASTStatement>(statements))) { }
  
  void ivisit(Visitor* visitor) override { condition->visit(visitor); statements->visit(visitor); }
};


struct IfBlock
{
  UniqueExpression condition;
  std::list<std::unique_ptr<ASTStatement>> body;
  
  IfBlock() { }
  
  IfBlock(std::list<ASTStatement*> body)
  {
    for (auto* s : body)
      this->body.push_back(std::unique_ptr<ASTStatement>(s));
  }
  
  IfBlock(ASTExpression* condition, std::list<ASTStatement*> body)
  {
    this->condition = UniqueExpression(condition);
    for (auto* s : body)
      this->body.push_back(std::unique_ptr<ASTStatement>(s));
  }
};

class ASTIf : public ASTStatement
{
private:
  std::list<IfBlock> blocks;
  IfBlock elseBody;
  
  std::string mnemonic() const override { return "If"; }
  
public:
  ASTIf(ASTExpression *condition, std::list<ASTStatement*> body)
  {
    blocks.push_back(IfBlock(condition, body));
  }
  
  ASTIf(ASTExpression *condition, std::list<ASTStatement*> body, std::list<ASTStatement*> fbody)
  {
    blocks.push_back(IfBlock(condition, body));
    elseBody = IfBlock(fbody);
  }
};


class ASTReturn : public ASTStatement
{
private:
  UniqueExpression value;
  
  std::string mnemonic() const override { return "Return"; }
  
public:
  ASTReturn(ASTExpression *value) : value(UniqueExpression(value)) { }
  ASTReturn() { }
  
  void ivisit(Visitor* visitor) override { value->visit(visitor); }
};



/*
 
 
 byte x;
 byte* x;

 word y;
 word* y;
 
 byte[10] x;
 word[15] x;
 
 string y = "antani"
 
*/

}

#endif