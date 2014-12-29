#ifndef __ENUMS_H__
#define __ENUMS_H__

#include "../utils.h"

#include <string>
#include <vector>
#include <unordered_map>

class Enum
{
private:
  std::string name;
  std::vector<std::string> entries;
  std::unordered_map<std::string, s32> values;
  
public:
  Enum(const std::string& name) : name(name)
  {
    
  }
  
  void add(const std::string& name, s32 value)
  {
    entries.push_back(name);
    values[name] = value;
  }
  
  void add(const std::string& name)
  {
    const auto& it = entries.back();
    s32 previousValue = values[it];
    
    entries.push_back(name);
    values[name] = previousValue + 1;
  }
  
  s32 retrieve(const std::string& name)
  {
    return values[name];
  }
};




#endif