#include "types.h"
#include "symbol_table.h"

using namespace nanoc;

u16 Named::getSize(const SymbolTable *table) const
{
  return table->getSizeForType(name);
}

bool Named::isStruct(const SymbolTable *table) const
{
  return table->isStructType(name);
}