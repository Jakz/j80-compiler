#include "compiler_exceptions.h"

#include "symbol_table.h"

using namespace nanoc;

std::string nanoc::compiler_exception::buffer;

const char* wrong_number_of_arguments::what() const throw()
{
  buffer = fmt::format("Exception at {}:{}, {}: {}, expecting {}, found {}", loc.begin.line, loc.begin.column, compiler_exception::what(), function.getName(), function.getArguments().size(), actualAmountOfArguments);
  return buffer.c_str();
}