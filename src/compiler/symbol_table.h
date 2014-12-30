#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#include <vector>
#include <unordered_map>

#include "enums.h"
#include "types.h"
#include "structs.h"

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
    FunctionSymbol(const FunctionSymbol& other) : name(other.name), returnType(UniqueType(other.returnType->copy())), arguments(other.arguments) { }
    FunctionSymbol& operator=(const FunctionSymbol& other) { this->name = other.name; this->returnType = UniqueType(other.returnType->copy()); this->arguments = arguments; return *this; }
    FunctionSymbol(const std::string& name, Type* returnType, std::vector<Symbol>& arguments) : name(name), returnType(UniqueType(returnType->copy())), arguments(arguments) { }
    
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

    UniqueTable table;
    LocalSymbolTable* currentTable;
    
  public:
    SymbolTable() : table(std::unique_ptr<LocalSymbolTable>(new LocalSymbolTable(nullptr))), currentTable(table.get())
    {

    }
  
    void addFunction(const std::string& name, Type* returnType, std::vector<Symbol>& arguments) { functions[name] = FunctionSymbol(name, returnType, arguments); }
    bool hasFunction(const std::string& name) { auto it = functions.find(name); return it != functions.end(); }
    
    void addSymbol(const std::string& name, Type* type) { currentTable->addSymbol(name, type); }
    
    void pushScope() {
      currentTable = currentTable->pushScope();
    }
    
    void popScope() {
      currentTable = currentTable->getParent();
    }
    
    Enum* addEnum(const std::string& name)
    {
      Enum* newEnum = new Enum(name);
      enums[name] = std::unique_ptr<Enum>(newEnum);
      
      return newEnum;
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
    
  public:
    SymbolsVisitor() : currentEnum(nullptr) { }
    void enteringNode(ASTScope* node);
    void exitingNode(ASTScope* node);
    void exitingNode(ASTFuncDeclaration* node);
    
    void enteringNode(ASTFuncDeclaration* node);
    void enteringNode(ASTDeclarationValue* node);
    void enteringNode(ASTDeclarationArray* node);
    
    void enteringNode(ASTEnumDeclaration* node);
    void exitingNode(ASTEnumDeclaration* node);
    void enteringNode(ASTEnumEntry* node);

    
    void commonVisit(ASTNode* node) { }
    
    const SymbolTable& getTable() { return table; }
  };
  
}



#endif