#ifndef __AST_H__
#define __AST_H__

#include <memory>
#include <vector>
#include <list>
#include <string>

#include "support/format/format.h"
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
  protected:
    location loc;
    
    ASTNode(const ASTNode&) = delete;
    ASTNode() = delete;
    
  public:
    ASTNode(const location& loc) : loc(loc) { }
    virtual ~ASTNode() { }

    virtual std::string mnemonic() const = 0;
    
    const location& getLocation() { return loc; }
  };
  
  template<typename T>
  class ASTList : public ASTNode
  {
  private:
    std::list<std::unique_ptr<T>> elements;
    
    std::string mnemonic() const override {
      /*std::string unmangledName = Utils::execute(std::string("c++filt ")+typeid(T).name());
      unmangledName.erase(unmangledName.end()-1);*/
      std::string unmangledName = typeid(T).name();
      size_t namespaceIndex = unmangledName.find("::");
      unmangledName = unmangledName.substr(namespaceIndex+2);
      return fmt::format("List<{}>", unmangledName);
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
    
    virtual bool isConstexpr() const { return false; }
    virtual Value getValue() const;
  };
  
  class ASTNumber : public ASTExpression
  {
  protected:
    Value value;
    
  public:
    ASTNumber(const location& loc, Value value) : ASTNode(loc), ASTExpression(loc), value(value) { }
    std::string mnemonic() const override { return fmt::format("Number({})", value); }
    
    //TODO: leak, manage width of immediate
    Type* getType(const SymbolTable& table) const override { return new Word(); }

    bool isConstexpr() const override { return true; }
    Value getValue() const override { return value; }
  };
  
  class ASTBool : public ASTExpression
  {
  protected:
    bool value;
    
  public:
    ASTBool(const location& loc, bool value) : ASTNode(loc), ASTExpression(loc), value(value) { }
    std::string mnemonic() const override { return value ? "true" : "false"; }
    Type* getType(const SymbolTable& table) const override { return new Bool(); }
  };
  
  class ASTReference : public ASTExpression
  {
  protected:
    std::string name;
    
  public:
    ASTReference(const location& loc, const std::string& name) : ASTNode(loc), ASTExpression(loc), name(name) { }
    std::string mnemonic() const override { return fmt::format("Reference({})", name.c_str()); }
    
    bool isConstexpr() const override;
    const std::string& getName() { return name; }
  };
  
  class ASTArrayReference : public ASTExpression
  {
  protected:
    UniqueExpression lhs;
    UniqueExpression index;
    
  public:
    ASTArrayReference(const location& loc, ASTExpression* lhs, ASTExpression* index) : ASTNode(loc), ASTExpression(loc), lhs(lhs), index(UniqueExpression(index)) { }
    std::string mnemonic() const override { return fmt::format("ArrayReference({})"); }
    
    UniqueExpression& getLeftHand() { return lhs; }
    std::unique_ptr<ASTExpression>& getIndex() { return index; }
  };
  
  class ASTCall : public ASTExpression
  {
  protected:
    std::string name;
    UniqueList<ASTExpression> arguments;
    
    
  public:
    ASTCall(const location& loc, const std::string& name) : ASTNode(loc), ASTExpression(loc), name(name) { }
    ASTCall(const location& loc, const std::string& name, std::list<ASTExpression*>& arguments) : ASTNode(loc), ASTExpression(loc), name(name),
      arguments(UniqueList<ASTExpression>(new ASTList<ASTExpression>(loc, arguments))) { }

    std::string mnemonic() const override { return fmt::format("Call({})", name.c_str()); }

    const std::string& getName() { return name; }
    std::unique_ptr<ASTList<ASTExpression>>& getArguments() { return arguments; }
  };
  
  class ASTTernaryExpression : public ASTExpression
  {
  protected:
    Ternary op;
    UniqueExpression operand1, operand2, operand3;
    
  public:
    ASTTernaryExpression(const location& loc, Ternary op, ASTExpression* operand1, ASTExpression* operand2, ASTExpression* operand3) : ASTNode(loc), ASTExpression(loc), op(op),
      operand1(UniqueExpression(operand1)), operand2(UniqueExpression(operand2)), operand3(UniqueExpression(operand3)) { }
    
    std::string mnemonic() const override { return fmt::format("TernaryExpression({})", Mnemonics::mnemonicForTernary(op)); }

    const Type* getType(const SymbolTable& table) const override;
    
    std::unique_ptr<ASTExpression>& getOperand1() { return operand1; }
    std::unique_ptr<ASTExpression>& getOperand2() { return operand2; }
    std::unique_ptr<ASTExpression>& getOperand3() { return operand3; }
  };
  
  
  class ASTBinaryExpression : public ASTExpression
  {
  protected:
    Binary op;
    UniqueExpression operand1, operand2;
        
  public:
    ASTBinaryExpression(const location& loc, Binary op, ASTExpression* operand1, ASTExpression* operand2) : ASTNode(loc), ASTExpression(loc), op(op),
      operand1(UniqueExpression(operand1)), operand2(UniqueExpression(operand2)) { }
    
    std::string mnemonic() const override { return fmt::format("BinaryExpression({})", Mnemonics::mnemonicForBinary(op)); }

    const Type* getType(const SymbolTable& table) const override;
    
    Binary getOperation() { return op; }
    std::unique_ptr<ASTExpression>& getOperand1() { return operand1; }
    std::unique_ptr<ASTExpression>& getOperand2() { return operand2; }
  };
  
  
  class ASTUnaryExpression : public ASTExpression
  {
  protected:
    Unary op;
    UniqueExpression operand;
       
  public:
    ASTUnaryExpression(const location& loc, Unary op, ASTExpression* operand) : ASTNode(loc), ASTExpression(loc), op(op), operand(operand) { }
   
    std::string mnemonic() const override { return fmt::format("UnaryExpression({})", Mnemonics::mnemonicForUnary(op)); }

    const Type* getType(const SymbolTable& table) const override;
    
    std::unique_ptr<ASTExpression>& getOperand() { return operand; }
  };
  
  class ASTFieldAccess : public ASTExpression
  {
  protected:
    bool isPointer;
    std::string field;
    UniqueExpression expression;
    
  public:
    ASTFieldAccess(const location& loc, ASTExpression* expression, const std::string& field, bool isPointer) : ASTNode(loc), ASTExpression(loc), expression(expression), field(field), isPointer(isPointer) { }
    std::string mnemonic() const override { return fmt::format("FieldAccess({})", field.c_str()); }
    UniqueExpression& getExpression() { return expression; }
    const Type* getType(const SymbolTable& table) const override;
  };
  
  class ASTDereference : public ASTExpression
  {
  protected:
    UniqueExpression expression;
  public:
    ASTDereference(const location& loc, ASTExpression* expression) : ASTNode(loc), ASTExpression(loc), expression(expression) { }
    std::string mnemonic() const override { return "Dereference"; }
    UniqueExpression& getExpression() { return expression; }

  };
  
  class ASTAddressOf : public ASTExpression
  {
  protected:
    UniqueExpression expression;
  public:
    ASTAddressOf(const location& loc, ASTExpression* expression) : ASTNode(loc), ASTExpression(loc), expression(expression) { }
    std::string mnemonic() const override { return "AddressOf"; }
    UniqueExpression& getExpression() { return expression; }

  };
  
  class ASTLeftHand : public ASTNode
  {
  protected:
    std::string name;
    
  public:
    ASTLeftHand(const location& loc, const std::string& name) : ASTNode(loc), name(name) { }
    
    std::string mnemonic() const override { return fmt::format("{}", name.c_str()); }
    const std::string& getName() { return name; }
  };
  
  class ASTScope : virtual public ASTStatement
  {
  protected:
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
  protected:
    std::unique_ptr<ASTLeftHand> leftHand;
    UniqueExpression expression;
    
  public:
    ASTAssign(const location& loc, ASTLeftHand *leftHand, ASTExpression* expression) : ASTNode(loc), ASTStatement(loc), leftHand(std::unique_ptr<ASTLeftHand>(leftHand)), expression(UniqueExpression(expression))
    {
      
    }
    
    std::unique_ptr<ASTLeftHand>& getLeftHand() { return leftHand; }
    std::unique_ptr<ASTExpression>& getRightHand() { return expression; }
    
    std::string mnemonic() const override { return fmt::format("Assign({})", leftHand->mnemonic().c_str()); }
  };


  class ASTVariableDeclaration : public ASTDeclaration
  {
  protected:
    ASTVariableDeclaration(const location& loc, const std::string& name) : ASTDeclaration(loc), name(name) { }
    
    std::string name;
    std::string mnemonic() const override { return fmt::format("Declaration({}, {})", name.c_str(), getTypeName().c_str()); }
    
  public:
    virtual Type* getType() const = 0;
    virtual std::string getTypeName() const = 0;
    
    const std::string& getName() { return name; }
  };

  class ASTDeclarationValue : public ASTVariableDeclaration
  {
  protected:
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
  protected:
    const u16 length;
    std::unique_ptr<Array> type;
    UniqueList<ASTExpression> initializer;
    
    std::string mnemonic() const override { return fmt::format("DeclarationArray({}, {}, {})", name.c_str(), getTypeName().c_str(), length); }
    
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
  protected:
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
      std::string mnemonic = fmt::format("FunctionDeclaration({}, {}", name, returnType->mnemonic());

      if (!arguments.empty())
      {
        mnemonic += ", [";
        bool first = true;
        for (const auto& arg : arguments)
        {
          if (!first) mnemonic += ", ";
          mnemonic += fmt::format("{} {}", arg.type->mnemonic(), arg.name);
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
protected:
  std::string name;
  bool hasValue;
  s32 value;
  
  std::string mnemonic() const override {
    if (hasValue)
      return fmt::format("EnumEntry({}, {})", name, value);
    else
      return fmt::format("EnumEntry({})", name);
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
protected:
  std::string name;
  UniqueList<ASTEnumEntry> entries;
  
  std::string mnemonic() const override { return fmt::format("EnumDeclaration({})", name); }
  
public:
  ASTEnumDeclaration(const location& loc, std::string name, const std::list<ASTEnumEntry*>& entries) : ASTNode(loc), ASTStatement(loc), ASTDeclaration(loc), name(name), entries(UniqueList<ASTEnumEntry>(new ASTList<ASTEnumEntry>(loc, entries))) { }

  std::unique_ptr<ASTList<ASTEnumEntry>>& getEntries() { return entries; }
  const std::string& getName() { return name; }
};

class ASTStructField : public ASTNode
{
protected:
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
protected:
  std::string name;
  UniqueList<ASTStructField> fields;
  
  std::string mnemonic() const override { return fmt::format("StructDeclaration({})", name.c_str()); }

public:
  ASTStructDeclaration(const location& loc, std::string name, const std::list<ASTStructField*>& fields) : ASTNode(loc), ASTStatement(loc), ASTDeclaration(loc), name(name), fields(UniqueList<ASTStructField>(new ASTList<ASTStructField>(loc, fields))) { }
  UniqueList<ASTStructField>& getFields() { return fields; }

  const std::string& getName() { return name; }

};



  class ASTWhile : public ASTStatement
  {
  protected:
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
  protected:
    std::unique_ptr<ASTStatement> body;
    
  public:
    ASTConditionalBlock(const location& loc, ASTStatement* body) : ASTNode(loc), body(std::unique_ptr<ASTStatement>(body)) { }
    std::unique_ptr<ASTStatement>& getBody() { return body; }
  };
  
  class ASTIfBlock : public ASTConditionalBlock
  {
  protected:
    std::unique_ptr<ASTExpression> condition;
    
    std::string mnemonic() const override { return "If"; }

    
  public:
    ASTIfBlock(const location& loc, ASTExpression* condition, ASTStatement* body) : ASTConditionalBlock(loc, body), condition(std::unique_ptr<ASTExpression>(condition)) { }
    
    std::unique_ptr<ASTExpression>& getCondition() { return condition; }
  };
  
  class ASTElseBlock : public ASTConditionalBlock
  {
  protected:
    std::string mnemonic() const override { return "Else"; }
    
  public:
    ASTElseBlock(const location& loc, ASTStatement* body) : ASTConditionalBlock(loc, body) { }
  };
  

class ASTConditional : public ASTStatement
{
protected:
  UniqueList<ASTConditionalBlock> blocks;

  std::string mnemonic() const override { return "Conditional"; }
  
public:
  ASTConditional(const location& loc, std::list<ASTConditionalBlock*>& blocks) : ASTNode(loc), ASTStatement(loc), blocks(UniqueList<ASTConditionalBlock>(new ASTList<ASTConditionalBlock>(loc, blocks))) { }
  
  std::unique_ptr<ASTList<ASTConditionalBlock>>& getBlocks() { return blocks; }
};


class ASTReturn : public ASTStatement
{
protected:
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