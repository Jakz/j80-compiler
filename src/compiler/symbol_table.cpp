#include "symbol_table.h"

#include "ast.h"

#include <iostream>

using namespace nanoc;
using namespace std;

u16 SymbolTable::getSizeForType(const std::string& name) const
{
  auto type = enums.find(name);
  
  if (type != enums.end())
    return type->second->getType()->getSize(this);
  
  auto type2 = structs.find(name);
  
  if (type2 != structs.end())
    return type2->second->getSize();
  
  return 0;
  /* TODO: error */
}

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
      
      cout << fmt::format("   {}, {} bytes at {:04X}",field->getName(),field->getType()->getSize(this),field->getOffset()) << endl;
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
  if (table.hasFunction(node->getName()))
    throw identifier_redefined(node->getLocation(), node->getName());
  
  table.pushScope();
  
  list<Argument> arguments = node->getArguments();
  vector<Symbol> sarguments;
  
  transform(arguments.begin(), arguments.end(), std::back_inserter(sarguments), [](const Argument& arg){ return Symbol(arg.name, arg.type.get()); } );
  for_each(arguments.begin(), arguments.end(), [this](const Argument& arg){ table.addSymbol(arg.name, arg.type.get()); });
  
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
  currentStruct->prepare(&table);
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

void SymbolsVisitor::enteringNode(ASTLeftHand* node)
{
  if (!table.isIdentifierBound(node->getName()) && !table.isEnumIdentifierBound(node->getName()))
    throw identifier_undeclared(node->getLocation(), node->getName());
}

void SymbolsVisitor::enteringNode(ASTReference* node)
{
  if (!table.isIdentifierBound(node->getName()) && !table.isEnumIdentifierBound(node->getName()))
    throw identifier_undeclared(node->getLocation(), node->getName());
}

#pragma mark EnumReplaceVisitor

ASTNode* EnumReplaceVisitor::exitingNode(ASTReference* node)
{
  const s32* const value = table.getValueForEnumEntry(node->getName());
  
  if (value)
    return new ASTNumber(node->getLocation(), *value);
  else
    return nullptr;
}

#pragma mark TypeCheckVisitor

void TypeCheckVisitor::enteringNode(ASTCall *node)
{
  const FunctionSymbol& symbol = table.getFunction(node->getName());
  
  if (node->getArguments()->getElements().size() != symbol.getArguments().size())
    throw wrong_number_of_arguments(node->getLocation(), symbol, node->getArguments()->getElements().size());
}