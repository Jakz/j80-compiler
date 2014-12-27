#include "symbol_table.h"

#include "ast.h"

#include <iostream>

using namespace nanoc;
using namespace std;

void SymbolTable::print() const
{
  cout << "Symbol Table" << endl;
  cout << " Functions: " << endl;
  for (const auto& f : functions)
  {
    const FunctionSymbol& function = f.second;
    cout << "  " << function.getName() << " " << Mnemonics::mnemonicForType(function.getReturnType()) << endl;
  }

  cout << " Symbols: " << endl;
  LocalSymbolTable* table = this->currentTable;
  printTable(table,1);

}

void SymbolTable::printTable(LocalSymbolTable *table, u16 scopes) const
{
  for (const auto& s : table->symbols)
  {
    const Symbol& symbol = s.second;
    cout << string(scopes*2, ' ') << symbol.getName() << " " << Mnemonics::mnemonicForType(symbol.getType()) << endl;
  }
  
  for (const auto& s : table->scopes)
  {
    printTable(s.get(), scopes+1);
  }
  
}



void SymbolsVisitor::enteringNode(ASTScope* node)
{
  table.pushScope();
}

void SymbolsVisitor::exitingNode(ASTScope* node)
{
  node->setSymbolTable(table.getCurrentTable());
  table.popScope();
}

void SymbolsVisitor::exitingNode(ASTFuncDeclaration* node)
{
  node->setSymbolTable(table.getCurrentTable());
  table.popScope();
}

void SymbolsVisitor::enteringNode(ASTFuncDeclaration *node)
{
  table.pushScope();
  
  list<Argument> arguments = node->getArguments();
  vector<Symbol> sarguments;
  
  transform(arguments.begin(), arguments.end(), std::back_inserter(sarguments), [](const Argument& arg){ return Symbol(arg.name, arg.type); } );
  
  table.addFunction(node->getName(), node->getReturnType(), sarguments);
}

void SymbolsVisitor::enteringNode(ASTDeclarationValue* node)
{
  table.addSymbol(node->getName(), node->getType());
}

void SymbolsVisitor::enteringNode(ASTDeclarationArray* node)
{
  table.addSymbol(node->getName(), node->getType());
}


