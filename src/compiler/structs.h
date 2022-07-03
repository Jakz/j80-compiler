#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include "types.h"
#include "../utils.h"
#include <string>

namespace nanoc
{

class StructField
{
private:
  u16 offset;
  std::string name;
  std::unique_ptr<const RealType> type;
  
public:
  StructField(const std::string& name, const RealType* type) : name(name), type(std::unique_ptr<const RealType>(type)), offset(0) { }
  void setOffset(u16 offset) { this->offset = offset; }
  
  const std::string& getName() const { return name; }
  u16 getOffset() { return offset; }
  const RealType* getType() { return type.get(); }
};

class Struct
{
private:
  std::string name;
  u16 size;
  std::vector<std::unique_ptr<StructField>> fields;
  
public:
  Struct(const std::string& name) : name(name) { }
  
  void addField(const std::string& name, const RealType *type)
  {
    fields.push_back(std::unique_ptr<StructField>(new StructField(name, type)));
  }
  
  const std::string& getName() { return name; }
  u16 getSize() { return size; }
  size_t getFieldCount() { return fields.size(); }
  
  const std::unique_ptr<StructField>& getField(size_t index) { return fields[index]; }
  
  void prepare(SymbolTable* table)
  {
    u16 offset = 0;
    
    for (auto& sf : fields)
    {
      sf->setOffset(offset);
      offset += sf->getType()->getSize(table);
    }
    
    this->size = offset;
  }
};


}

#endif