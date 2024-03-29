#ifndef __ENUMS_H__
#define __ENUMS_H__

#include "utils.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace nanoc {
  
class Type;

class Enum
{
private:
  std::string name;
  std::vector<std::string> entries;
  std::unordered_map<std::string, s32> values;
  std::unique_ptr<Type> type;
  
public:
  Enum() = default;
  Enum(const std::string& name) : name(name)
  {
    
  }
  
  void setType(Type* type) { this->type = std::unique_ptr<Type>(type); }
  
  void add(const std::string& name, s32 value)
  {
    entries.push_back(name);
    values[name] = value;
  }
  
  void add(const std::string& name)
  {
    s32 assignedValue = !entries.empty() ? values[entries.back()] + 1: 0;
    
    entries.push_back(name);
    values[name] = assignedValue;
  }
  
  const s32 * const retrieve(const std::string& name)
  {
    auto it = values.find(name);
    
    return it != values.end() ? &it->second : nullptr;
  }
  
  const Type* getType() { return type.get(); }
  const std::string& getName() const { return name; }
  const size_t size() const { return entries.size(); }
  const std::string& getEnumName(size_t index) const { return entries[index]; }
  s32 getEnumValue(size_t index) const { return values.find(entries.at(index))->second; }
  
  
};

}

#endif
