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
  std::unique_ptr<RealType> type;
  
public:
  StructField(const std::string& name, RealType* type) : name(name), type(std::unique_ptr<RealType>(type)), offset(0) { }
  void setOffset(u16 offset) { this->offset = offset; }
  
  const std::string& getName() const { return name; }
  u16 getOffset() { return offset; }
  RealType* getType() { return type.get(); }
};

class Struct
{
private:
  u16 size;
  std::vector<std::unique_ptr<StructField>> fields;
  
public:
  void addField(const std::string& name, RealType *type)
  {
    fields.push_back(std::unique_ptr<StructField>(new StructField(name, type)));
  }
  
  u16 getSize() { return size; }
  
  void prepare()
  {
    u16 offset = 0;
    
    for (auto& sf : fields)
    {
      sf->setOffset(offset);
      offset += sf->getType()->getSize();
    }
    
    this->size = offset;
  }
};


}

#endif