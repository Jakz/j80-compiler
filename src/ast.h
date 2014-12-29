#ifndef __AST_H__
#define __AST_H__

#include <memory>
#include <vector>
#include <list>
#include <string>

#include "format.h"
#include "utils.h"
#include "ast_visitor.h"
#include "compiler/types.h"

namespace nanoc
{
  typedef signed int Value;

  struct Argument
  {
    std::string name;
    std::unique_ptr<BaseType> type;
    
    Argument(std::string name, BaseType* type) : name(name), type(std::unique_ptr<BaseType>(type)) { }
    Argument& operator=(const Argument& other) { this->name = other.name; this->type = std::unique_ptr<BaseType>(static_cast<BaseType*>(other.type->copy())); return *this; }
    Argument(const Argument& other) : name(other.name), type(std::unique_ptr<BaseType>(static_cast<BaseType*>(other.type->copy()))) { }
  };

  class LocalSymbolTable;
  
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
  public:
    void accept(Visitor* visitor) { visitor->visit(this); }
    
    virtual std::string mnemonic() const = 0;
  };
  
  template<typename T>
  class ASTList : public ASTNode
  {
  private:
    std::list<std::unique_ptr<T>> elements;
    
    std::string mnemonic() const override {
      std::string unmangledName = Utils::execute(std::string("c++filt ")+typeid(T).name());
      unmangledName.erase(unmangledName.end()-1);
      size_t namespaceIndex = unmangledName.find("::");
      unmangledName = unmangledName.substr(namespaceIndex+2);
      return fmt::sprintf("List<%s>", unmangledName.c_str());
    }
    
  public:
    ASTList() { }
    ASTList(std::list<T*>& elements)
    {
      for (auto* s : elements)
        this->elements.push_back(std::unique_ptr<T>(s));
    }
    
    const std::list<std::unique_ptr<T>>& getElements() { return elements; }
  };

  class ASTStatement : virtual public ASTNode { };
  class ASTDeclaration : virtual public ASTStatement { };
  
  
  
  class ASTExpression : public ASTStatement
  {
  public:
    // TODO: make pure virtual and implement in subtypes
    virtual Type* getType() const { return nullptr; }
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
  
  class ASTArrayReference : public ASTExpression
  {
  private:
    std::string name;
    UniqueExpression index;
    
  public:
    ASTArrayReference(const std::string& name, ASTExpression* index) : name(name), index(UniqueExpression(index)) { }
    std::string mnemonic() const override { return fmt::sprintf("ArrayReference(%s)", name.c_str()); }
    
    ASTExpression* getIndex() { return index.get(); }
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
    
    ASTList<ASTExpression>* getArguments() { return arguments.get(); }
  };
  
  class ASTTernaryExpression : public ASTExpression
  {
  private:
    Ternary op;
    UniqueExpression operand1, operand2, operand3;
    
    std::string mnemonic() const override { return fmt::sprintf("TernaryExpression(%s)", Mnemonics::mnemonicForTernary(op)); }
    
  public:
    ASTTernaryExpression(Ternary op, ASTExpression* operand1, ASTExpression* operand2, ASTExpression* operand3) : op(op),
      operand1(UniqueExpression(operand1)), operand2(UniqueExpression(operand2)), operand3(UniqueExpression(operand3)) { }
    
    ASTExpression* getOperand1() { return operand1.get(); }
    ASTExpression* getOperand2() { return operand2.get(); }
    ASTExpression* getOperand3() { return operand3.get(); }
  };
  
  
  class ASTBinaryExpression : public ASTExpression
  {
  private:
    Binary op;
    UniqueExpression operand1, operand2;
    
    std::string mnemonic() const override { return fmt::sprintf("BinaryExpression(%s)", Mnemonics::mnemonicForBinary(op)); }
    
  public:
    ASTBinaryExpression(Binary op, ASTExpression* operand1, ASTExpression* operand2) : op(op), operand1(UniqueExpression(operand1)), operand2(UniqueExpression(operand2)) { }
    
    ASTExpression* getOperand1() { return operand1.get(); }
    ASTExpression* getOperand2() { return operand2.get(); }
  };
  
  
  class ASTUnaryExpression : public ASTExpression
  {
  private:
    Unary op;
    UniqueExpression operand;
    
    std::string mnemonic() const override { return fmt::sprintf("UnaryExpression(%s)", Mnemonics::mnemonicForUnary(op)); }
    
  public:
    ASTUnaryExpression(Unary op, ASTExpression* operand) : op(op), operand(UniqueExpression(operand)) { }
    
    ASTExpression* getOperand() { return operand.get(); }
  };

  class ASTLeftHand : public ASTNode
  {
  private:
    std::string name;
    
  public:
    ASTLeftHand(const std::string& name) : name(name) { }
    
    std::string mnemonic() const override { return fmt::sprintf("%s", name.c_str()); }
  };
  
  class ASTScope : virtual public ASTStatement
  {
  private:
    UniqueList<ASTStatement> statements;
    LocalSymbolTable* symbols;
    
    std::string mnemonic() const override { return "Scope"; }

    
  public:
    ASTScope(std::list<ASTStatement*>& statements) : statements(UniqueList<ASTStatement>(new ASTList<ASTStatement>(statements))) { }
    
    ASTList<ASTStatement>* getStatements() { return statements.get(); }
    
    void setSymbolTable(LocalSymbolTable *table) { symbols = table; }
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
    
    ASTExpression* getRightHand() { return expression.get(); }
    
    std::string mnemonic() const override { return fmt::sprintf("Assign(%s)", leftHand->mnemonic().c_str()); }
  };


  class ASTVariableDeclaration : public ASTDeclaration
  {
  protected:
    ASTVariableDeclaration(const std::string& name) : name(name) { }
    
    std::string name;
    std::string mnemonic() const override { return fmt::sprintf("Declaration(%s, %s)", name.c_str(), getTypeName().c_str()); }
    
  public:
    virtual Type* getType() const = 0;
    virtual std::string getTypeName() const = 0;
    
    const std::string& getName() { return name; }
  };

  class ASTDeclarationValue : public ASTVariableDeclaration
  {
  private:
    std::unique_ptr<RealType> type;
    UniqueExpression value;
  public:
    ASTDeclarationValue(const std::string& name, RealType* type, ASTExpression* value = nullptr) : ASTVariableDeclaration(name), type(std::unique_ptr<RealType>(type)), value(UniqueExpression(value)) { }
    Type* getType() const override { return type.get(); }
    std::string getTypeName() const override  { return type->mnemonic(); }
    ASTExpression* getInitializer() { return value.get(); }
  };

  class ASTDeclarationArray : public ASTVariableDeclaration
  {
  private:
    const u16 length;
    std::unique_ptr<Array> type;
    UniqueList<ASTExpression> initializer;
    
    std::string mnemonic() const override { return fmt::sprintf("DeclarationArray(%s, %s, %u)", name.c_str(), getTypeName().c_str(), length); }
    
  public:
    ASTDeclarationArray(const std::string& name, Array* type, u16 length) : ASTVariableDeclaration(name), type(std::unique_ptr<Array>(type)), length(length) { }
    ASTDeclarationArray(const std::string& name, Array* type, u16 length, std::list<ASTExpression*>& initializer) : ASTVariableDeclaration(name), type(std::unique_ptr<Array>(type)), length(length), initializer(UniqueList<ASTExpression>(new ASTList<ASTExpression>(initializer))) { }
    Type* getType() const override { return type.get(); }
    std::string getTypeName() const override { return type->mnemonic(); }
    
    ASTList<ASTExpression>* getInitializer() { return initializer.get(); }
  };

  class ASTDeclarationPtr : public ASTVariableDeclaration
  {
  private:
    u16 address;
    std::unique_ptr<Pointer> type;
  public:
    ASTDeclarationPtr(const std::string& name, Pointer* type, u16 address = 0) : ASTVariableDeclaration(name), type(std::unique_ptr<Pointer>(type)), address(address) { }
    Type* getType() const override { return type.get(); }
    Type* getItemType() const { return type->innerType(); }
    std::string getTypeName() const override  { return type->mnemonic(); }

  };

  class ASTFuncDeclaration : virtual public ASTScope, virtual public ASTDeclaration
  {
    std::string name;
    std::unique_ptr<BaseType> returnType;
    std::list<Argument> arguments;
    
    std::string mnemonic() const override
    {
      std::string mnemonic = fmt::sprintf("FunctionDeclaration(%s, %s", name.c_str(), returnType->mnemonic().c_str());

      if (!arguments.empty())
      {
        mnemonic += ", [";
        bool first = true;
        for (const auto& arg : arguments)
        {
          if (!first) mnemonic += ", ";
          mnemonic += fmt::sprintf("%s %s", arg.type->mnemonic(), arg.name.c_str());
          first = false;
        }
        mnemonic += "]";
      }
      mnemonic += ")";
      
      return mnemonic;
    }
    
  public:
    ASTFuncDeclaration(std::string name, BaseType* returnType, std::list<Argument>& arguments, std::list<ASTStatement*>& body) : ASTScope(body), name(name), returnType(std::unique_ptr<BaseType>(returnType)),
    arguments(std::move(arguments)) { }
  
    const std::string& getName() { return name; }
    Type* getReturnType() { return returnType.get(); }
    const std::list<Argument>& getArguments() { return arguments; }
  };

class ASTEnumEntry : public ASTNode
{
private:
  std::string name;
  bool hasValue;
  s32 value;
  
  std::string mnemonic() const override {
    if (hasValue)
      return fmt::sprintf("EnumEntry(%s, %d)", name.c_str(), value);
    else
      return fmt::sprintf("EnumEntry(%s)", name.c_str());
  }
  
public:
  ASTEnumEntry(const std::string& name, s32 value) : name(name), value(value), hasValue(true) { }
  ASTEnumEntry(const std::string& name) : name(name), hasValue(false) { }
  
  const std::string& getName() { return name; }
  bool getHasValue() { return hasValue; }
  s32 getValue() { return value; }
};

class ASTEnumDeclaration : virtual public ASTDeclaration
{
private:
  std::string name;
  UniqueList<ASTEnumEntry> entries;
  
  std::string mnemonic() const override { return fmt::sprintf("EnumDeclaration(%s)", name.c_str()); }
  
public:
  ASTEnumDeclaration(std::string name, std::list<ASTEnumEntry*>& entries) : name(name), entries(UniqueList<ASTEnumEntry>(new ASTList<ASTEnumEntry>(entries))) { }

  ASTList<ASTEnumEntry>* getEntries() { return entries.get(); }
  const std::string& getName() { return name; }
};




  class ASTWhile : public ASTStatement
  {
  private:
    UniqueExpression condition;
    std::unique_ptr<ASTStatement> body;
    
    std::string mnemonic() const override { return "While"; }
    
  public:
    ASTWhile(ASTExpression* condition, ASTStatement* body) : condition(UniqueExpression(condition)),
      body(std::unique_ptr<ASTStatement>(body)) { }
    
    ASTExpression* getCondition() { return condition.get(); }
    ASTStatement* getBody() { return body.get(); }
  };

  class ASTConditionalBlock : public ASTNode
  {
  private:
    std::unique_ptr<ASTStatement> body;
    
  public:
    ASTConditionalBlock(ASTStatement* body) : body(std::unique_ptr<ASTStatement>(body)) { }
    ASTStatement* getBody() { return body.get(); }
  };
  
  class ASTIfBlock : public ASTConditionalBlock
  {
  private:
    std::unique_ptr<ASTExpression> condition;
    
    std::string mnemonic() const override { return "If"; }

    
  public:
    ASTIfBlock(ASTExpression* condition, ASTStatement* body) : ASTConditionalBlock(body), condition(std::unique_ptr<ASTExpression>(condition)) { }
    
    ASTExpression* getCondition() { return condition.get(); }
  };
  
  class ASTElseBlock : public ASTConditionalBlock
  {
  private:
    std::string mnemonic() const override { return "Else"; }
    
  public:
    ASTElseBlock(ASTStatement* body) : ASTConditionalBlock(body) { }
  };
  

class ASTConditional : public ASTStatement
{
private:
  UniqueList<ASTConditionalBlock> blocks;

  std::string mnemonic() const override { return "Conditional"; }
  
public:
  ASTConditional(std::list<ASTConditionalBlock*>& blocks) : blocks(UniqueList<ASTConditionalBlock>(new ASTList<ASTConditionalBlock>(blocks))) { }
  
  ASTList<ASTConditionalBlock>* getBlocks() { return blocks.get(); }
};


class ASTReturn : public ASTStatement
{
private:
  UniqueExpression value;
  
  std::string mnemonic() const override { return "Return"; }
  
public:
  ASTReturn(ASTExpression *value = nullptr) : value(UniqueExpression(value)) { }  
  ASTExpression* getValue() { return value.get(); }
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