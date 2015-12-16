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
    cout << "  " << function.getName() << " " << function.getReturnType()->mnemonic() << endl;
  }
  
  
  cout << " Enums: " << endl;
  for (const auto& e : enums)
  {
    cout << "  " << e.second->getName() << endl;
    
    for (int i = 0; i < e.second->size(); ++i)
      cout << "   " << e.second->getEnumName(i) << " = " << e.second->getEnumValue(i) << endl;
  }
  
  cout << " Structs: " << endl;
  for (const auto& s : structs)
  {
    cout << "  " << s.second->getName() << ", size: " << s.second->getSize() << endl;
    
    for (int i = 0; i < s.second->getFieldCount(); ++i)
    {
      const auto& field = s.second->getField(i);
      
      cout << fmt::sprintf("   %s, %u bytes at %04X",field->getName(),field->getType()->getSize(),field->getOffset()) << endl;
    }
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
    cout << string(scopes*2, ' ') << symbol.getName() << " " << symbol.getType()->mnemonic() << endl;
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

ASTNode* SymbolsVisitor::exitingNode(ASTScope* node)
{
  node->setSymbolTable(table.getCurrentTable());
  table.popScope();
  return nullptr;
}

ASTNode* SymbolsVisitor::exitingNode(ASTFuncDeclaration* node)
{
  node->setSymbolTable(table.getCurrentTable());
  table.popScope();
  return nullptr;
}

void SymbolsVisitor::enteringNode(ASTFuncDeclaration *node)
{
  table.pushScope();
  
  list<Argument> arguments = node->getArguments();
  vector<Symbol> sarguments;
  
  transform(arguments.begin(), arguments.end(), std::back_inserter(sarguments), [](const Argument& arg){ return Symbol(arg.name, arg.type.get()); } );
  
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

void SymbolsVisitor::enteringNode(ASTEnumDeclaration *node)
{
  if (table.isNameFree(node->getName()))
  {
    currentEnum = table.addEnum(node->getName());
  }
}

ASTNode* SymbolsVisitor::exitingNode(ASTEnumDeclaration* node)
{
  currentEnum = nullptr;
  return nullptr;
}

void SymbolsVisitor::enteringNode(ASTStructField* node)
{
  currentStruct->addField(node->getName(), node->getType());
}

void SymbolsVisitor::enteringNode(ASTStructDeclaration *node)
{
  if (table.isNameFree(node->getName()))
  {
    currentStruct = table.addStruct(node->getName());
  }
}

ASTNode* SymbolsVisitor::exitingNode(ASTStructDeclaration* node)
{
  currentStruct->prepare();
  currentStruct = nullptr;
  return nullptr;
}

void SymbolsVisitor::enteringNode(ASTEnumEntry* node)
{
  if (node->getHasValue())
    currentEnum->add(node->getName(), node->getValue());
  else
    currentEnum->add(node->getName());
}

ASTNode* EnumReplaceVisitor::exitingNode(ASTReference* node)
{
  const s32* const value = table.getValueForEnumEntry(node->getName());
  return value ? new ASTNumber(*value) : nullptr;
}


