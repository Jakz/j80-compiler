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
    BYTE,
    WORD,
    BYTE_PTR,
    WORD_PTR,
    BYTE_ARRAY,
    WORD_ARRAY,
    VOID
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


  class ASTNode
  {
    virtual void ivisit(Visitor* visitor) { }
    
  public:
    void visit(Visitor* visitor)
    {
      visitor->enteringNode(this);
      visitor->visit(this);
      ivisit(visitor);
      visitor->exitingNode(this);
    }
    
    virtual std::string mnemonic() const = 0;
    
    virtual void recursivePrint(u16 pad) const;
    void printPad(u16 pad) const;
  };
  
  
  class ASTStatement : public ASTNode { };
  
  
  
  class ASTExpression : public ASTStatement
  {
  public:
    // TODO: make pure and implement in subtypes
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
    std::list<UniqueExpression> arguments;
    
    std::string mnemonic() const override { return fmt::sprintf("Call(%s)", name.c_str()); }
    
  public:
    ASTCall(const std::string& name) : name(name) { }
    ASTCall(const std::string& name, std::list<ASTExpression*> arguments) : name(name)
    {
      for (auto* argument : arguments)
        this->arguments.push_back(std::unique_ptr<ASTExpression>(argument));
    }
    
    void ivisit(Visitor* visitor) override { for (const auto& a : arguments) a->visit(visitor); }

    
    void recursivePrint(u16 pad) const override
    {
      ASTExpression::recursivePrint(pad);
      
      if (!arguments.empty())
      {
        for (const auto& arg : arguments)
          arg->recursivePrint(pad+1);
      }
      
    }
  };
  
  
  class ASTBinaryExpression : public ASTExpression
  {
  private:
    Binary op;
    UniqueExpression operand1, operand2;
    
    std::string mnemonic() const override { return fmt::sprintf("BinaryExpression(%s)", nameForType(op)); }
    
  public:
    ASTBinaryExpression(Binary op, ASTExpression* operand1, ASTExpression* operand2) : op(op), operand1(UniqueExpression(operand1)), operand2(UniqueExpression(operand2)) { }
    
    void ivisit(Visitor* visitor) override { operand1->visit(visitor); operand2->visit(visitor); }

    
    void recursivePrint(u16 pad) const override
    {
      ASTNode::recursivePrint(pad);
      operand1->recursivePrint(pad+1);
      operand2->recursivePrint(pad+1);
    }
    
    static const char* nameForType(Binary op)
    {
      switch (op) {
        case Binary::ADDITION: return "+";
        case Binary::SUBTRACTION: return "-";
        case Binary::AND: return "&";
        case Binary::OR: return "|";
        case Binary::XOR: return "^";
        case Binary::EQ: return "==";
        case Binary::NEQ: return "!=";
        case Binary::GREATEREQ: return ">=";
        case Binary::LESSEQ: return "<=";
        case Binary::GREATER: return ">";
        case Binary::LESS: return "<";
        default: return "";
      }
    }
    
  };
  
  
  class ASTUnaryExpression : public ASTExpression
  {
  private:
    Unary op;
    UniqueExpression operand;
    
    std::string mnemonic() const override { return fmt::sprintf("UnaryExpression(%s)", nameForType(op)); }
    
  public:
    ASTUnaryExpression(Unary op, ASTExpression* operand) : op(op), operand(UniqueExpression(operand)) { }
    
    void recursivePrint(u16 pad) const override
    {
      ASTNode::recursivePrint(pad);
      operand->recursivePrint(pad+1);
    }
    
    void ivisit(Visitor* visitor) override { operand->visit(visitor); }
    
    static const char* nameForType(Unary op)
    {
      switch (op) {
        case Unary::NOT: return "!";
        case Unary::INCR: return "++";
        case Unary::DECR: return "--";
        case Unary::NEG: return "-";
        default: return "";
      }
    }
  };
    
  class ASTList { };

  class ASTListRecur : public ASTNode, public ASTList
  {
  private:
    std::unique_ptr<ASTListRecur> next;
    std::unique_ptr<ASTNode> item;
    
  public:
    ASTListRecur(ASTNode* item, ASTListRecur* next = nullptr) :
      next(std::unique_ptr<ASTListRecur>(next)), item(std::unique_ptr<ASTNode>(item)) { }
    
    ASTListRecur* getNext() { return next.get(); }
    std::unique_ptr<ASTNode> stealItem() { return std::move(item); }
    
    std::string mnemonic() const override { return "List"; }
    void ivisit(Visitor* v) { item->visit(v); next->visit(v); }

    void recursivePrint(u16 pad) const override;
    
  };

  class ASTListSeq : public ASTNode, public ASTList
  {
  private:
    std::list<std::unique_ptr<ASTNode>> children;

    std::string mnemonic() const override { return "List"; }
    
  public:
    void recursivePrint(u16 pad) const override;
    
    void prepend(UniqueNode item) { children.push_front(std::move(item)); }
    void append(UniqueNode item) { children.push_back(std::move(item)); }

    void ivisit(Visitor* v) { for (const auto& c : children) c->visit(v); }

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

    void recursivePrint(u16 pad) const override { ASTStatement::recursivePrint(pad); expression->recursivePrint(pad+1); }
  };


  class ASTDeclaration : public ASTNode
  {
  private:
    std::string name;
    
  protected:
    ASTDeclaration(const std::string& name) : name(name) { }
    
    std::string mnemonic() const override { return fmt::sprintf("Declaration(%s, %s)", name.c_str(), getTypeName().c_str()); }
    
  public:
    virtual Type getType() const = 0;
    virtual std::string getTypeName() const = 0;
    
    static const char* nameForType(Type t)
    {
      switch (t) {
        case Type::BYTE: return "byte";
        case Type::WORD: return "word";
        case Type::BYTE_PTR: return "byte*";
        case Type::WORD_PTR: return "word*";
        case Type::BYTE_ARRAY: return "byte[]";
        case Type::WORD_ARRAY: return "word[]";
        case Type::VOID: return "void";
      }
    }
  };
    
  class ASTDeclarationByte : public ASTDeclaration
  {
  private:
    UniqueExpression value; // TODO: if a signed value is stored here print will be incorrect
  public:
    ASTDeclarationByte(const std::string& name, ASTExpression* value = nullptr) : ASTDeclaration(name), value(UniqueExpression(value)) { }
    Type getType() const override { return Type::BYTE; }
    std::string getTypeName() const override  { return "byte"; }
    
    void ivisit(Visitor* visitor) override { value->visit(visitor); }

    
    void recursivePrint(u16 pad) const override
    {
      ASTDeclaration::recursivePrint(pad);
      if (value)
        value->recursivePrint(pad+1);
    }

  };

  class ASTDeclarationWord : public ASTDeclaration
  {
  private:
    UniqueExpression value;
  public:
    ASTDeclarationWord(const std::string& name, ASTExpression* value = nullptr) : ASTDeclaration(name), value(UniqueExpression(value)) { }
    Type getType() const override { return Type::WORD; }
    std::string getTypeName() const override  { return "word"; }
    
    void ivisit(Visitor* visitor) override { value->visit(visitor); }

    void recursivePrint(u16 pad) const override
    {
      ASTDeclaration::recursivePrint(pad);
      if (value)
        value->recursivePrint(pad+1);
    }
  };

  class ASTDeclarationPtr : public ASTDeclaration
  {
  private:
    u16 address;
    Type type;
  public:
    ASTDeclarationPtr(const std::string& name, Type type, u16 address = 0) : ASTDeclaration(name), type(type), address(address) { }
    Type getType() const override { return type; }
    Type getItemType() const { return type == Type::WORD_PTR ? Type::WORD : Type::BYTE; }
    std::string getTypeName() const override  { return type == Type::WORD_PTR ? "word*" : "byte*"; }

  };

  class ASTDeclarationArray : public ASTDeclaration
  {
  private:
    u16 length;
    Type type;
  public:
    ASTDeclarationArray(const std::string& name, Type type, u16 length = 0) : ASTDeclaration(name), type(type), length(length) { }
    Type getType() const override { return type; }
    Type getItemType() const { return type == Type::WORD_PTR ? Type::WORD : Type::BYTE; }
    std::string getTypeName() const override  { return  std::string(type == Type::WORD_PTR ? "word[" : "byte[")+std::to_string(length)+"]"; }
  };

  class ASTFuncDeclaration : public ASTNode
  {
    std::string name;
    Type returnType;
    std::list<Argument> arguments;
    std::list<std::unique_ptr<ASTStatement>> statements;
    
    std::string mnemonic() const override
    {
      std::string mnemonic = fmt::sprintf("FunctionDeclaration(%s, %s", name.c_str(), ASTDeclaration::nameForType(returnType));

      if (!arguments.empty())
      {
        mnemonic += ", [";
        bool first = true;
        for (const auto& arg : arguments)
        {
          if (!first) mnemonic += ", ";
          mnemonic += fmt::sprintf("%s %s", ASTDeclaration::nameForType(arg.type), arg.name.c_str());
          first = false;
        }
        mnemonic += "]";
      }
      mnemonic += ")";
      
      return mnemonic;
    }
    
  public:
    ASTFuncDeclaration(std::string name, Type returnType, std::list<Argument>& arguments, std::list<ASTStatement*>& statements) : name(name), returnType(returnType), arguments(arguments)
    {
      for (auto* statement : statements)
        this->statements.push_back(std::unique_ptr<ASTStatement>(statement));
    }
    
    void recursivePrint(u16 pad) const override
    {
      ASTNode::recursivePrint(pad);
      for (const auto& statement : statements)
        statement->recursivePrint(pad+1);
    }
    
    void ivisit(Visitor* visitor) override { for (const auto& s : statements) s->visit(visitor); }

  };


class ASTWhile : public ASTStatement
{
private:
  UniqueExpression condition;
  std::list<std::unique_ptr<ASTStatement>> statements;
  
  std::string mnemonic() const override { return "While"; }
  
public:
  ASTWhile(ASTExpression* condition, std::list<ASTStatement*> statements) : condition(UniqueExpression(condition))
  {
    for (auto* statement : statements)
      this->statements.push_back(std::unique_ptr<ASTStatement>(statement));
  }
  
  void recursivePrint(u16 pad) const override
  {
    ASTNode::recursivePrint(pad);
    condition->recursivePrint(pad+1);
    for (const auto& statement : statements)
      statement->recursivePrint(pad+1);
  }
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
    
  void recursivePrint(u16 pad) const override
  {
    ASTStatement::recursivePrint(pad);
    printPad(pad+1);
    printf("Condition\n");
    const IfBlock& trueBlock = blocks.front();
    
    trueBlock.condition->recursivePrint(pad+2);
    
    printPad(pad+1);
    printf("Body\n");
    for (const auto& s : trueBlock.body)
      s->recursivePrint(pad+2);
    
    if (!elseBody.body.empty())
    {
      printPad(pad+1);
      printf("Else Body\n");
      
      for (const auto& s : elseBody.body)
        s->recursivePrint(pad+2);
    }
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
  
  void recursivePrint(u16 pad) const override
  {
    ASTStatement::recursivePrint(pad);
    if (value)
      value->recursivePrint(pad+1);
  }
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