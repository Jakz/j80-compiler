#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#include <vector>
#include <unordered_map>

#include "../format.h"
#include "../ast_visitor.h"
#include "../utils.h"


namespace nanoc
{
  class LocalSymbolTable;
  using UniqueTable = std::unique_ptr<LocalSymbolTable>;
  
  class Symbol
  {
    std::string name;
    Type type;
    
  public:
    Symbol() = default;
    Symbol(const std::string& name, Type type) : name(name), type(type) { }
    
    const std::string mnemonic() { return fmt::sprintf("Symbol(%s, %s)", name, Mnemonics::mnemonicForType(type)); }
    
    const std::string& getName() const { return name; }
    const Type getType() const { return type; }
  };
  
  class FunctionSymbol
  {
    std::string name;
    Type returnType;
    std::vector<Symbol> arguments;
    
  public:
    FunctionSymbol() = default;
    FunctionSymbol(const std::string& name, Type returnType, std::vector<Symbol>& arguments) : name(name), returnType(returnType), arguments(arguments) { }
    
    const std::string mnemonic() { return fmt::sprintf("Function(%s, %s)", name, Mnemonics::mnemonicForType(returnType)); }

    
    const std::string& getName() const { return name; }
    const Type getReturnType() const { return returnType; }
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
    
    bool hasSymbol(const std::string& name) { return std::find_if(symbols.begin(), symbols.end(), [&](const std::pair<std::string,Symbol>& p) { return p.first == name; }) != symbols.end(); }
    void addSymbol(const std::string& name, Type type) { symbols[name] = Symbol(name, type); }
    
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

    UniqueTable table;
    LocalSymbolTable* currentTable;
    
  public:
    SymbolTable() : table(std::unique_ptr<LocalSymbolTable>(new LocalSymbolTable(nullptr))), currentTable(table.get())
    {

    }
  
    void addFunction(const std::string& name, Type returnType, std::vector<Symbol>& arguments) { functions[name] = FunctionSymbol(name, returnType, arguments); }
    bool hasFunction(const std::string& name) { return std::find_if(functions.begin(), functions.end(), [&](const std::pair<std::string,FunctionSymbol>& p) { return p.first == name; }) != functions.end(); }
    
    void addSymbol(const std::string& name, Type type) { currentTable->addSymbol(name, type); }
    
    void pushScope() {
      currentTable = currentTable->pushScope();
    }
    
    void popScope() {
      currentTable = currentTable->getParent();
    }
    
    void print() const;
    void printTable(LocalSymbolTable* table, u16 scopes = 0) const;
    
    LocalSymbolTable* getCurrentTable() { return currentTable; }
  };
  
  
  
  class SymbolsVisitor : public Visitor
  {
  private:
    SymbolTable table;
    
  public:
    SymbolsVisitor() { }
    void enteringNode(ASTScope* node);
    void exitingNode(ASTScope* node);
    void exitingNode(ASTFuncDeclaration* node);
    
    void enteringNode(ASTFuncDeclaration* node);
    void enteringNode(ASTDeclarationValue<Type::BOOL>* node);
    void enteringNode(ASTDeclarationValue<Type::WORD>* node);
    void enteringNode(ASTDeclarationValue<Type::BYTE>* node);

    
    void commonVisit(ASTNode* node) { }
    
    const SymbolTable& getTable() { return table; }
  };
  
}



#endif