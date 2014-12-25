#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#include <vector>
#include <unordered_map>

#include "../utils.h"


namespace nanoc
{
  class Symbol
  {
    std::string name;
    Type type;
    
  public:
    Symbol() = default;
    Symbol(const std::string& name, Type type) : name(name), type(type) { }
    
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
    
    const std::string& getName() const { return name; }
    const Type getReturnType() const { return returnType; }
    const std::vector<Symbol>& getArguments() const { return arguments; }

  };
  
  class LocalSymbolTable
  {
    std::unordered_map<std::string, Symbol> symbols;
    
  public:
    LocalSymbolTable() = default;
    
    bool hasSymbol(const std::string& name) { return std::find_if(symbols.begin(), symbols.end(), [&](const std::pair<std::string,Symbol>& p) { return p.first == name; }) != symbols.end(); }
    void addSymbol(const std::string& name, Type type) { symbols[name] = Symbol(name, type); }
    

  };
  
  class SymbolTable
  {
  private:
    std::unordered_map<std::string, FunctionSymbol> functions;
    std::vector<LocalSymbolTable> scopes;
    std::vector<LocalSymbolTable>::iterator currentScope;
    
    
  public:
    SymbolTable()
    {
      scopes.push_back(LocalSymbolTable());
      currentScope = scopes.begin();
    }
  
    void addFunction(const std::string& name, Type returnType, std::vector<Symbol>& arguments) { functions[name] = FunctionSymbol(name, returnType, arguments); }
    bool hasFunction(const std::string& name) { return std::find_if(functions.begin(), functions.end(), [&](const std::pair<std::string,FunctionSymbol>& p) { return p.first == name; }) != functions.end(); }
    
    void addSymbol(const std::string& name, Type type) { currentScope->addSymbol(name, type); }
    
    void pushScope() {
      scopes.push_back(LocalSymbolTable());
      ++currentScope;
    }
    
    void popScope() {
      scopes.pop_back();
    }
  };
  
  
}



#endif