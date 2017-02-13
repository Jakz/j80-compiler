#include "instruction.h"

#include "assembler.h"

using namespace Assembler;


Result InstructionLD_NN::solve(const Environment& env)
{
  switch (value.type)
  {
    case Value8::Type::VALUE: break;
    case Value8::Type::DATA_LENGTH:
    {
      auto it = env.data.map.find(value.label);
      
      if (it == env.data.map.end())
        return Result(fmt::sprintf("reference to missing data '%s'.", value.label.c_str()));
      
      u16 cvalue = it->second.length;
      
      if (!valueFitsType<dest_t>(cvalue))
        return Result(fmt::sprintf("Error: constant %s has a value too large for destination (%u).", value.label.c_str(), cvalue));
      
      env.assembler.log(Log::INFO, true, "  > Data '%s' length: %u", value.label.c_str(), it->second.length);
      value.value = it->second.length;
      break;
    }
    case Value8::Type::CONST:
    {
      auto it = env.consts.find(value.label);
      
      if (it == env.consts.end())
        return Result(fmt::sprintf("reference to missing const '%s'.", value.label.c_str()));
      
      u16 cvalue = it->second;
      
      if (!valueFitsType<dest_t>(cvalue))
        return Result(fmt::sprintf("Error: constant %s has a value too large for destination (%u).", value.label.c_str(), cvalue));
      
      env.assembler.log(Log::INFO, true, "  > Data '%s' value", value.label.c_str());
      value.value = it->second;
      break;
    }
  }
      
  return Result();
}
