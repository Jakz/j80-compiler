#ifndef __AST_H__
#define __AST_H__

#include <memory>
#include <vector>
#include <list>
#include <string>

#include "utils.h"

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
    virtual void print() const = 0;
    
  public:
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
    u16 value;
    
  public:
    ASTNumber(u16 value) : value(value) { }
    
    void print() const override {printf("Number(%d)", value); }
  };
  
  class ASTReference : public ASTExpression
  {
  private:
    std::string name;
    
  public:
    ASTReference(const std::string& name) : name(name) { }
    void print() const override { printf("Reference(%s)", name.c_str()); }
  };
  
  class ASTCall : public ASTExpression
  {
  private:
    std::string name;
    std::list<UniqueExpression> arguments;
    
    void print() const override { printf("Call(%s)", name.c_str()); }
    
  public:
    ASTCall(const std::string& name) : name(name) { }
    ASTCall(const std::string& name, std::list<ASTExpression*> arguments) : name(name)
    {
      for (auto* argument : arguments)
        this->arguments.push_back(std::unique_ptr<ASTExpression>(argument));
    }
    
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
    
    void print() const override { printf("BinaryExpression(%s)", nameForType(op)); }
    
  public:
    ASTBinaryExpression(Binary op, ASTExpression* operand1, ASTExpression* operand2) : op(op), operand1(UniqueExpression(operand1)), operand2(UniqueExpression(operand2)) { }
    
    
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
    
    virtual void print() const override { printf("UnaryExpression(%s)", nameForType(op)); }
    
  public:
    ASTUnaryExpression(Unary op, ASTExpression* operand) : op(op), operand(UniqueExpression(operand)) { }
    
    void recursivePrint(u16 pad) const override
    {
      ASTNode::recursivePrint(pad);
      operand->recursivePrint(pad+1);
    }
    
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
    
    void print() const override { printf("List"); }

    void recursivePrint(u16 pad) const override;
    
  };

  class ASTListSeq : public ASTNode, public ASTList
  {
  private:
    std::list<std::unique_ptr<ASTNode>> children;

    void print() const override { printf("List"); }
    
  public:
    void recursivePrint(u16 pad) const override;
    
    void prepend(UniqueNode item) { children.push_front(std::move(item)); }
    void append(UniqueNode item) { children.push_back(std::move(item)); }
  };

  class ASTLeftHand : public ASTNode
  {
  private:
    std::string name;
    
  public:
    ASTLeftHand(const std::string& name) : name(name) { }
    
    void print() const override { printf("%s", name.c_str()); }
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
    
    void print() const override { printf("Assign("); leftHand->print(); printf(")"); }
    void recursivePrint(u16 pad) const override { ASTStatement::recursivePrint(pad); expression->recursivePrint(pad+1); }
  };


  class ASTDeclaration : public ASTNode
  {
  private:
    std::string name;
    
  protected:
    ASTDeclaration(const std::string& name) : name(name) { }
    
    void print() const override { printf("Declaration(%s, %s)", name.c_str(), getTypeName().c_str()); }
    
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
    u8 value;
  public:
    ASTDeclarationByte(const std::string& name, u8 value = 0) : ASTDeclaration(name), value(value) { }
    Type getType() const override { return Type::BYTE; }
    std::string getTypeName() const override  { return "byte"; }
  };

  class ASTDeclarationWord : public ASTDeclaration
  {
  private:
    u16 value;
  public:
    ASTDeclarationWord(const std::string& name, u16 value = 0) : ASTDeclaration(name), value(value) { }
    Type getType() const override { return Type::WORD; }
    std::string getTypeName() const override  { return "word"; }
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
    
    void print() const override
    {
      printf("FunctionDeclaration(%s, %s, ", name.c_str(), ASTDeclaration::nameForType(returnType));
      if (!arguments.empty())
      {
        printf("[");
        bool first = true;
        for (const auto& arg : arguments)
        {
          if (!first) printf(", ");
          printf("%s %s", ASTDeclaration::nameForType(arg.type), arg.name.c_str());
          first = false;
        }
        printf("]");
      }
      printf(")");
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
  };


class ASTWhile : public ASTStatement
{
private:
  UniqueExpression condition;
  std::list<std::unique_ptr<ASTStatement>> statements;
  
  void print() const override { printf("While"); }
  
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
  
  void print() const override { printf("If"); }
  
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
  
  void print() const override { printf("Return"); }
  
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