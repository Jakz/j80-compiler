#include "compiler_exceptions.h"

#include "symbol_table.h"

using namespace nanoc;

std::string nanoc::compiler_exception::buffer;

const char* wrong_number_of_arguments::what() const throw()
{
  buffer = fmt::sprintf("Exception at %u:%u, %s: %s, expecting %u, found %u", loc.begin.line, loc.begin.column, compiler_exception::what(), function.getName(), function.getArguments().size(), actualAmountOfArguments);
  return buffer.c_str();
}