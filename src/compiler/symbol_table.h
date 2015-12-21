#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#include <vector>
#include <unordered_map>

#include "enums.h"
#include "types.h"
#include "structs.h"
#include "compiler_exceptions.h"

#include "../format.h"
#include "../ast_visitor.h"
#include "../utils.h"

namespace nanoc
{
  class LocalSymbolTable;
  using UniqueTable = std::unique_ptr<LocalSymbolTable>;
  
  using UniqueType = std::unique_ptr<Type>;
    
  class Symbol
  {
    std::string name;
    UniqueType type;
    
  public:
    Symbol() = default;
    Symbol(const Symbol& other) : name(other.name), type(UniqueType(other.type->copy())) { }
    Symbol& operator=(const Symbol& other) { this->name = other.name; this->type = UniqueType(other.type->copy()); return *this; }
    Symbol(const std::string& name, Type* type) : name(name), type(UniqueType(type->copy())) { }
    
    const std::string mnemonic() { return fmt::sprintf("Symbol(%s, %s)", name, type->mnemonic().c_str()); }
    
    const std::string& getName() const { return name; }
    const Type* getType() const { return type.get(); }
  };
  
  class FunctionSymbol
  {
    std::string name;
    UniqueType returnType;
    std::vector<Symbol> arguments;
    
  public:
    FunctionSymbol() = default;
    FunctionSymbol(const FunctionSymbol& other) = delete;
    FunctionSymbol& operator=(const FunctionSymbol& other) { this->name = other.name; this->returnType = UniqueType(other.returnType->copy()); this->arguments = other.arguments; return *this; }
    FunctionSymbol(const std::string& name, Type* returnType, const std::vector<Symbol>& arguments) : name(name), returnType(UniqueType(returnType->copy())), arguments(arguments) { }
    
    const std::string mnemonic() { return fmt::sprintf("Function(%s, %s)", name, returnType->mnemonic().c_str()); }

    
    const std::string& getName() const { return name; }
    const Type* getReturnType() const { return returnType.get(); }
    const std::vector<Symbol>& getArguments() const { return arguments; }

  };
  
  class LocalSymbolTable
  {
    std::unordered_map<std::string, Symbol> symbols;
    std::vector<UniqueTable> scopes;
    LocalSymbolTable* parent;
    
  public:
    LocalSymbolTable(LocalSymbolTable* parent) : parent(parent) { }
    LocalSymbolTable() = default;
    
    bool hasSymbol(const std::string& name) { auto it = symbols.find(name); return it != symbols.end(); }
    void addSymbol(const std::string& name, Type* type) { symbols[name] = Symbol(name, type); }
    
    const Type* find(const std::string& name)
    {
      const auto it = symbols.find(name);
      
      if (it != symbols.end())
        return it->second.getType();
      else
      {
        if (parent)
          return parent->find(name);
        else
          return nullptr;
      }
    }
    
    LocalSymbolTable* pushScope()
    {
      scopes.push_back(UniqueTable(new LocalSymbolTable(this)));
      return scopes.back().get();
    }
    
    LocalSymbolTable* getParent() { return parent; }

    friend class SymbolTable;
  };
  
  class SymbolTable
  {
  private:
    std::unordered_map<std::string, FunctionSymbol> functions;
    std::unordered_map<std::string, std::unique_ptr<Enum>> enums;
    std::unordered_map<std::string, std::unique_ptr<Struct>> structs;

    UniqueTable table;
    LocalSymbolTable* currentTable;
    
  public:
    SymbolTable() : table(std::unique_ptr<LocalSymbolTable>(new LocalSymbolTable(nullptr))), currentTable(table.get())
    {

    }
  
    void addFunction(const std::string& name, Type* returnType, const std::vector<Symbol>& arguments) { functions[name] = FunctionSymbol(name, returnType, arguments); }
    bool hasFunction(const std::string& name) { auto it = functions.find(name); return it != functions.end(); }
    const FunctionSymbol& getFunction(const std::string& name) const { return functions.find(name)->second; }
    
    void addSymbol(const std::string& name, Type* type) { currentTable->addSymbol(name, type); }
    
    void pushScope() {
      currentTable = currentTable->pushScope();
    }
    
    void popScope() {
      currentTable = currentTable->getParent();
    }

    bool isNameFree(const std::string& name) const
    {
      return enums.find(name) == enums.end() && !isStructType(name);
    }
    
    bool isIdentifierBound(const std::string& name) const
    {
      return currentTable->find(name) != nullptr;
    }
    
    bool isEnumIdentifierBound(const std::string& name) const
    {
      return std::any_of(std::begin(enums), std::end(enums), [name](const decltype(enums)::value_type& e) { return e.second->retrieve(name) == nullptr; });
    }
    
    bool isStructType(const std::string& name) const { return structs.find(name) != structs.end(); }
        
    u16 getSizeForType(const std::string& name) const;
    
    Enum* addEnum(const std::string& name)
    {
      Enum* newEnum = new Enum(name);
      enums[name] = std::unique_ptr<Enum>(newEnum);
      
      return newEnum;
    }
    
    Struct* addStruct(const std::string& name)
    {
      Struct* newStruct = new Struct(name);
      structs[name] = std::unique_ptr<Struct>(newStruct);
      return newStruct;
    }
    
    const s32* const getValueForEnumEntry(const std::string& name) const
    {
      for (const auto& e : enums)
      {
        const s32* const value = e.second->retrieve(name);
        
        if (value)
          return value;
      }
      
      return nullptr;
    }
    
    void print() const;
    void printTable(LocalSymbolTable* table, u16 scopes = 0) const;
    
    LocalSymbolTable* getCurrentTable() { return currentTable; }
  };
  
  
  
  class SymbolsVisitor : public Visitor
  {
  private:
    SymbolTable table;
    Enum *currentEnum;
    Struct* currentStruct;
    
  public:
    SymbolsVisitor() : currentEnum(nullptr), currentStruct(nullptr) { }
    void enteringNode(ASTScope* node);
    ASTNode* exitingNode(ASTScope* node);
    ASTNode* exitingNode(ASTFuncDeclaration* node);
    
    void enteringNode(ASTFuncDeclaration* node);
    void enteringNode(ASTDeclarationValue* node);
    void enteringNode(ASTDeclarationArray* node);
    
    void enteringNode(ASTEnumDeclaration* node);
    ASTNode* exitingNode(ASTEnumDeclaration* node);
    void enteringNode(ASTEnumEntry* node);

    void enteringNode(ASTStructDeclaration* node);
    ASTNode* exitingNode(ASTStructDeclaration* node);
    void enteringNode(ASTStructField* node);
    
    void enteringNode(ASTReference* node);
    void enteringNode(ASTLeftHand* node);
    
    
    void commonVisit(ASTNode* node) { }
    
    const SymbolTable& getTable() { return table; }
  };
  
  class EnumReplaceVisitor : public Visitor
  {
  private:
    const SymbolTable& table;
    
  public:
    EnumReplaceVisitor(const SymbolTable& table) : table(table) { }
    
    ASTNode* exitingNode(ASTReference* node);
  };
  
  class TypeCheckVisitor : public Visitor
  {
  private:
    const SymbolTable& table;
    
  public:
    TypeCheckVisitor(const SymbolTable& table) : table(table) { }
    
    void enteringNode(ASTCall* node) override;
  };
  
}



#endif