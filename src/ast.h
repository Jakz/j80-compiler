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

#include "compiler/location.hh"


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
  private:
    location loc;
    
    ASTNode(const ASTNode&) = delete;
    ASTNode() = delete;
    
  public:
    ASTNode(const location& loc) : loc(loc) { }
    virtual std::string mnemonic() const = 0;
    
    const location& getLocation() { return loc; }
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
    ASTList(const location& loc, const std::list<T*>& elements) : ASTNode(loc)
    {
      for (auto* s : elements)
        this->elements.push_back(std::unique_ptr<T>(s));
    }
    
    std::list<std::unique_ptr<T>>& getElements() { return elements; }
  };

  class ASTStatement : virtual public ASTNode
  {
  protected:
    ASTStatement(const location& loc) : ASTNode(loc) { }
  };
  
  class ASTDeclaration : virtual public ASTStatement
  {
  protected:
    ASTDeclaration(const location& loc) : ASTStatement(loc) { }
  };
  
  
  
  class ASTExpression : public ASTStatement
  {
  public:
    ASTExpression(const location& loc) : ASTStatement(loc) { }

    // TODO: make pure virtual and implement in subtypes
    virtual const Type* getType(const SymbolTable& table) const { return nullptr; /* TODO: used to make it compile */ }; // = 0;
  };
  
  class ASTNumber : public ASTExpression
  {
  private:
    Value value;
    
  public:
    ASTNumber(const location& loc, Value value) : ASTNode(loc), ASTExpression(loc), value(value) { }
    std::string mnemonic() const override { return fmt::sprintf("Number(%d)", value); }
    Value getValue() const { return value; }
    
    //TODO: leak, manage width of immediate
    Type* getType(const SymbolTable& table) const override { return new Word(); }
  };
  
  class ASTBool : public ASTExpression
  {
  private:
    bool value;
    
  public:
    ASTBool(const location& loc, bool value) : ASTNode(loc), ASTExpression(loc), value(value) { }
    std::string mnemonic() const override { return value ? "true" : "false"; }
    Type* getType(const SymbolTable& table) const override { return new Bool(); }
  };
  
  class ASTReference : public ASTExpression
  {
  private:
    std::string name;
    
  public:
    ASTReference(const location& loc, const std::string& name) : ASTNode(loc), ASTExpression(loc), name(name) { }
    std::string mnemonic() const override { return fmt::sprintf("Reference(%s)", name.c_str()); }
    
    const std::string& getName() { return name; }
  };
  
  class ASTArrayReference : public ASTExpression
  {
  private:
    UniqueExpression lhs;
    UniqueExpression index;
    
  public:
    ASTArrayReference(const location& loc, ASTExpression* lhs, ASTExpression* index) : ASTNode(loc), ASTExpression(loc), lhs(lhs), index(UniqueExpression(index)) { }
    std::string mnemonic() const override { return fmt::sprintf("ArrayReference(%s)"); }
    
    UniqueExpression& getLeftHand() { return lhs; }
    std::unique_ptr<ASTExpression>& getIndex() { return index; }
  };
  
  class ASTCall : public ASTExpression
  {
  private:
    std::string name;
    UniqueList<ASTExpression> arguments;
    
    std::string mnemonic() const override { return fmt::sprintf("Call(%s)", name.c_str()); }
    
  public:
    ASTCall(const location& loc, const std::string& name) : ASTNode(loc), ASTExpression(loc), name(name) { }
    ASTCall(const location& loc, const std::string& name, std::list<ASTExpression*>& arguments) : ASTNode(loc), ASTExpression(loc), name(name),
      arguments(UniqueList<ASTExpression>(new ASTList<ASTExpression>(loc, arguments))) { }
    
    const std::string& getName() { return name; }
    std::unique_ptr<ASTList<ASTExpression>>& getArguments() { return arguments; }
  };
  
  class ASTTernaryExpression : public ASTExpression
  {
  private:
    Ternary op;
    UniqueExpression operand1, operand2, operand3;
    
    std::string mnemonic() const override { return fmt::sprintf("TernaryExpression(%s)", Mnemonics::mnemonicForTernary(op)); }
    
  public:
    ASTTernaryExpression(const location& loc, Ternary op, ASTExpression* operand1, ASTExpression* operand2, ASTExpression* operand3) : ASTNode(loc), ASTExpression(loc), op(op),
      operand1(UniqueExpression(operand1)), operand2(UniqueExpression(operand2)), operand3(UniqueExpression(operand3)) { }
    
    const Type* getType(const SymbolTable& table) const override;
    
    std::unique_ptr<ASTExpression>& getOperand1() { return operand1; }
    std::unique_ptr<ASTExpression>& getOperand2() { return operand2; }
    std::unique_ptr<ASTExpression>& getOperand3() { return operand3; }
  };
  
  
  class ASTBinaryExpression : public ASTExpression
  {
  private:
    Binary op;
    UniqueExpression operand1, operand2;
    
    std::string mnemonic() const override { return fmt::sprintf("BinaryExpression(%s)", Mnemonics::mnemonicForBinary(op)); }
    
  public:
    ASTBinaryExpression(const location& loc, Binary op, ASTExpression* operand1, ASTExpression* operand2) : ASTNode(loc), ASTExpression(loc), op(op),
      operand1(UniqueExpression(operand1)), operand2(UniqueExpression(operand2)) { }
    
    const Type* getType(const SymbolTable& table) const override;
    
    Binary getOperation() { return op; }
    std::unique_ptr<ASTExpression>& getOperand1() { return operand1; }
    std::unique_ptr<ASTExpression>& getOperand2() { return operand2; }
  };
  
  
  class ASTUnaryExpression : public ASTExpression
  {
  private:
    Unary op;
    UniqueExpression operand;
    
    std::string mnemonic() const override { return fmt::sprintf("UnaryExpression(%s)", Mnemonics::mnemonicForUnary(op)); }
    
  public:
    ASTUnaryExpression(const location& loc, Unary op, ASTExpression* operand) : ASTNode(loc), ASTExpression(loc), op(op), operand(operand) { }
    
    const Type* getType(const SymbolTable& table) const override;
    
    std::unique_ptr<ASTExpression>& getOperand() { return operand; }
  };
  
  class ASTFieldAccess : public ASTExpression
  {
  private:
    bool isPointer;
    std::string field;
    UniqueExpression expression;
    
  public:
    ASTFieldAccess(const location& loc, ASTExpression* expression, const std::string& field, bool isPointer) : ASTNode(loc), ASTExpression(loc), expression(expression), field(field), isPointer(isPointer) { }
    std::string mnemonic() const override { return fmt::sprintf("FieldAccess(%s)", field.c_str()); }
    UniqueExpression& getExpression() { return expression; }
    const Type* getType(const SymbolTable& table) const override;
  };
  
  class ASTDereference : public ASTExpression
  {
  private:
    UniqueExpression expression;
  public:
    ASTDereference(const location& loc, ASTExpression* expression) : ASTNode(loc), ASTExpression(loc), expression(expression) { }
    std::string mnemonic() const override { return "Dereference"; }
    UniqueExpression& getExpression() { return expression; }

  };
  
  class ASTAddressOf : public ASTExpression
  {
  private:
    UniqueExpression expression;
  public:
    ASTAddressOf(const location& loc, ASTExpression* expression) : ASTNode(loc), ASTExpression(loc), expression(expression) { }
    std::string mnemonic() const override { return "AddressOf"; }
    UniqueExpression& getExpression() { return expression; }

  };
  
  class ASTLeftHand : public ASTNode
  {
  private:
    std::string name;
    
  public:
    ASTLeftHand(const location& loc, const std::string& name) : ASTNode(loc), name(name) { }
    
    std::string mnemonic() const override { return fmt::sprintf("%s", name.c_str()); }
    const std::string& getName() { return name; }
  };
  
  class ASTScope : virtual public ASTStatement
  {
  private:
    UniqueList<ASTStatement> statements;
    LocalSymbolTable* symbols;
    
    std::string mnemonic() const override { return "Scope"; }

    
  public:
    ASTScope(const location& loc, std::list<ASTStatement*>& statements) : ASTNode(loc), ASTStatement(loc), statements(UniqueList<ASTStatement>(new ASTList<ASTStatement>(loc, statements))) { }
    
    std::unique_ptr<ASTList<ASTStatement>>& getStatements() { return statements; }
    
    void setSymbolTable(LocalSymbolTable *table) { symbols = table; }
  };

  class ASTAssign : public ASTStatement
  {
  private:
    std::unique_ptr<ASTLeftHand> leftHand;
    UniqueExpression expression;
    
  public:
    ASTAssign(const location& loc, ASTLeftHand *leftHand, ASTExpression* expression) : ASTNode(loc), ASTStatement(loc), leftHand(std::unique_ptr<ASTLeftHand>(leftHand)), expression(UniqueExpression(expression))
    {
      
    }
    
    std::unique_ptr<ASTLeftHand>& getLeftHand() { return leftHand; }
    std::unique_ptr<ASTExpression>& getRightHand() { return expression; }
    
    std::string mnemonic() const override { return fmt::sprintf("Assign(%s)", leftHand->mnemonic().c_str()); }
  };


  class ASTVariableDeclaration : public ASTDeclaration
  {
  protected:
    ASTVariableDeclaration(const location& loc, const std::string& name) : ASTDeclaration(loc), name(name) { }
    
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
    ASTDeclarationValue(const location& loc, const std::string& name, RealType* type, ASTExpression* value = nullptr) : ASTNode(loc), ASTStatement(loc), ASTVariableDeclaration(loc, name),
    type(std::unique_ptr<RealType>(type)), value(UniqueExpression(value)) { }
    Type* getType() const override { return type.get(); }
    std::string getTypeName() const override  { return type->mnemonic(); }
    std::unique_ptr<ASTExpression>& getInitializer() { return value; }
  };

  class ASTDeclarationArray : public ASTVariableDeclaration
  {
  private:
    const u16 length;
    std::unique_ptr<Array> type;
    UniqueList<ASTExpression> initializer;
    
    std::string mnemonic() const override { return fmt::sprintf("DeclarationArray(%s, %s, %u)", name.c_str(), getTypeName().c_str(), length); }
    
  public:
    ASTDeclarationArray(const location& loc, const std::string& name, Array* type, u16 length) : ASTNode(loc), ASTStatement(loc), ASTVariableDeclaration(loc, name),
      type(std::unique_ptr<Array>(type)), length(length) { }
    ASTDeclarationArray(const location& loc, const std::string& name, Array* type, u16 length, std::list<ASTExpression*>& initializer) : ASTNode(loc), ASTStatement(loc), ASTVariableDeclaration(loc, name),
      type(std::unique_ptr<Array>(type)), length(length), initializer(UniqueList<ASTExpression>(new ASTList<ASTExpression>(loc, initializer))) { }
    Type* getType() const override { return type.get(); }
    std::string getTypeName() const override { return type->mnemonic(); }
    
    std::unique_ptr<ASTList<ASTExpression>>& getInitializer() { return initializer; }
  };

  class ASTDeclarationPtr : public ASTVariableDeclaration
  {
  private:
    u16 address;
    std::unique_ptr<Pointer> type;
  public:
    ASTDeclarationPtr(const location& loc, const std::string& name, Pointer* type, u16 address = 0) : ASTNode(loc), ASTStatement(loc), ASTVariableDeclaration(loc, name), type(std::unique_ptr<Pointer>(type)), address(address) { }
    Type* getType() const override { return type.get(); }
    const Type* getItemType() const { return type->innerType(); }
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
    ASTFuncDeclaration(const location& loc, std::string name, BaseType* returnType, std::list<Argument>& arguments, std::list<ASTStatement*>& body) : ASTNode(loc), ASTDeclaration(loc), ASTStatement(loc), ASTScope(loc, body), name(name), returnType(std::unique_ptr<BaseType>(returnType)),
    arguments(std::move(arguments)) { }
  
    const std::string& getName() { return name; }
    BaseType* getReturnType() { return returnType.get(); }
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
  ASTEnumEntry(const location& loc, const std::string& name, s32 value) : ASTNode(loc), name(name), value(value), hasValue(true) { }
  ASTEnumEntry(const location& loc, const std::string& name) : ASTNode(loc), name(name), hasValue(false) { }
  
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
  ASTEnumDeclaration(const location& loc, std::string name, const std::list<ASTEnumEntry*>& entries) : ASTNode(loc), ASTStatement(loc), ASTDeclaration(loc), name(name), entries(UniqueList<ASTEnumEntry>(new ASTList<ASTEnumEntry>(loc, entries))) { }

  std::unique_ptr<ASTList<ASTEnumEntry>>& getEntries() { return entries; }
  const std::string& getName() { return name; }
};

class ASTStructField : public ASTNode
{
private:
  std::unique_ptr<ASTVariableDeclaration> entry;
  
public:
  ASTStructField(const location& loc, ASTVariableDeclaration* entry) : ASTNode(loc), entry(entry) { }
  
  std::string mnemonic() const override { return "StructField"; }
  
  const std::string& getName() { return entry->getName(); }
  const RealType* getType() { return static_cast<RealType*>(entry->getType()); }
  std::string getTypeName() { return entry->getTypeName(); }
  
  std::unique_ptr<ASTVariableDeclaration>& getDeclaration() { return entry; }
};


class ASTStructDeclaration : virtual public ASTDeclaration
{
private:
  std::string name;
  UniqueList<ASTStructField> fields;
  
  std::string mnemonic() const override { return fmt::sprintf("StructDeclaration(%s)", name.c_str()); }

public:
  ASTStructDeclaration(const location& loc, std::string name, const std::list<ASTStructField*>& fields) : ASTNode(loc), ASTStatement(loc), ASTDeclaration(loc), name(name), fields(UniqueList<ASTStructField>(new ASTList<ASTStructField>(loc, fields))) { }
  UniqueList<ASTStructField>& getFields() { return fields; }

  const std::string& getName() { return name; }

};



  class ASTWhile : public ASTStatement
  {
  private:
    UniqueExpression condition;
    std::unique_ptr<ASTStatement> body;
    
    std::string mnemonic() const override { return "While"; }
    
  public:
    ASTWhile(const location& loc, ASTExpression* condition, ASTStatement* body) : ASTNode(loc), ASTStatement(loc), condition(UniqueExpression(condition)),
      body(std::unique_ptr<ASTStatement>(body)) { }
    
    std::unique_ptr<ASTExpression>& getCondition() { return condition; }
    std::unique_ptr<ASTStatement>& getBody() { return body; }
  };

  class ASTConditionalBlock : public ASTNode
  {
  private:
    std::unique_ptr<ASTStatement> body;
    
  public:
    ASTConditionalBlock(const location& loc, ASTStatement* body) : ASTNode(loc), body(std::unique_ptr<ASTStatement>(body)) { }
    std::unique_ptr<ASTStatement>& getBody() { return body; }
  };
  
  class ASTIfBlock : public ASTConditionalBlock
  {
  private:
    std::unique_ptr<ASTExpression> condition;
    
    std::string mnemonic() const override { return "If"; }

    
  public:
    ASTIfBlock(const location& loc, ASTExpression* condition, ASTStatement* body) : ASTConditionalBlock(loc, body), condition(std::unique_ptr<ASTExpression>(condition)) { }
    
    std::unique_ptr<ASTExpression>& getCondition() { return condition; }
  };
  
  class ASTElseBlock : public ASTConditionalBlock
  {
  private:
    std::string mnemonic() const override { return "Else"; }
    
  public:
    ASTElseBlock(const location& loc, ASTStatement* body) : ASTConditionalBlock(loc, body) { }
  };
  

class ASTConditional : public ASTStatement
{
private:
  UniqueList<ASTConditionalBlock> blocks;

  std::string mnemonic() const override { return "Conditional"; }
  
public:
  ASTConditional(const location& loc, std::list<ASTConditionalBlock*>& blocks) : ASTNode(loc), ASTStatement(loc), blocks(UniqueList<ASTConditionalBlock>(new ASTList<ASTConditionalBlock>(loc, blocks))) { }
  
  std::unique_ptr<ASTList<ASTConditionalBlock>>& getBlocks() { return blocks; }
};


class ASTReturn : public ASTStatement
{
private:
  UniqueExpression value;
  
  std::string mnemonic() const override { return "Return"; }
  
public:
  ASTReturn(const location& loc, ASTExpression *value = nullptr) : ASTNode(loc), ASTStatement(loc), value(UniqueExpression(value)) { }
  std::unique_ptr<ASTExpression>& getValue() { return value; }
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