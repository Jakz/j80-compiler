#pragma once

#include <exception>
#include <string>

#include "location.hh"
#include "../format.h"

namespace nanoc
{
  class symbol_exception : public std::runtime_error
  {
  protected:
    static std::string buffer;
    location loc;
    
  public:
    symbol_exception(const location& loc, const std::string& s) : std::runtime_error(s), loc(loc) { }
  };

  class identifier_redefined : public symbol_exception
  {
  private:
    std::string identifier;
  public:
    identifier_redefined(const location& loc, const std::string& identifier) : symbol_exception(loc, "identifier already defined"), identifier(identifier) { }
    const char* what() const throw() override
    {
      buffer = fmt::sprintf("Exception at %u:%u, %s: %s", loc.begin.line, loc.begin.column, symbol_exception::what(), identifier.c_str());
      return buffer.c_str();
    }
  };
  
  class identifier_undeclared : public symbol_exception
  {
  private:
    std::string identifier;
  public:
    identifier_undeclared(const location& loc, const std::string& identifier) : symbol_exception(loc, "identifier undeclared"), identifier(identifier) { }
    const char* what() const throw() override
    {
      buffer = fmt::sprintf("Exception at %u:%u, %s: %s", loc.begin.line, loc.begin.column, symbol_exception::what(), identifier.c_str());
      return buffer.c_str();
    }
  };

}