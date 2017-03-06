#pragma once

#include <exception>
#include <string>

#include "location.hh"
#include "support/format.h"

namespace nanoc
{
  class FunctionSymbol;
  
  class compiler_exception : public std::runtime_error
  {
  protected:
    static std::string buffer;
    location loc;
    
  public:
    compiler_exception(const location& loc, const std::string& s) : std::runtime_error(s), loc(loc) { }
  };

  class identifier_redefined : public compiler_exception
  {
  private:
    std::string identifier;
  public:
    identifier_redefined(const location& loc, const std::string& identifier) : compiler_exception(loc, "identifier already defined"), identifier(identifier) { }
    const char* what() const throw() override
    {
      buffer = fmt::sprintf("Exception at %u:%u, %s: %s", loc.begin.line, loc.begin.column, compiler_exception::what(), identifier.c_str());
      return buffer.c_str();
    }
  };
  
  class identifier_undeclared : public compiler_exception
  {
  private:
    std::string identifier;
  public:
    identifier_undeclared(const location& loc, const std::string& identifier) : compiler_exception(loc, "identifier undeclared"), identifier(identifier) { }
    const char* what() const throw() override
    {
      buffer = fmt::sprintf("Exception at %u:%u, %s: %s", loc.begin.line, loc.begin.column, compiler_exception::what(), identifier.c_str());
      return buffer.c_str();
    }
  };
  
  class wrong_number_of_arguments : public compiler_exception
  {
  private:
    const FunctionSymbol& function;
    size_t actualAmountOfArguments;
    
  public:
    wrong_number_of_arguments(const location& loc, const FunctionSymbol& function, size_t arguments) : compiler_exception(loc, "wrong amount of arguments"),
    function(function), actualAmountOfArguments(arguments) { }
    
    const char* what() const throw() override;
  };

}
