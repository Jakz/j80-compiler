#ifndef __NANOC_COMPILER_H__
#define __NANOC_COMPILER_H__


#include "opcodes.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>

#include "compiler/nanoclexer.h"
#include "compiler/nanocparser.hpp"
#include "compiler/location.hh"

template<typename T, typename V>
using hash_map = std::unordered_map<T,V>;
using UniqueNode = std::unique_ptr<nanoc::ASTNode>;

namespace nanoc
{
  class SymbolTable
  {
    std::unordered_map<std::string, Type> variables;
  };
  
  
  class Compiler
  {
    private:
      hash_map<std::string, Type> globalVariables;
    
      std::unique_ptr<ASTNode> ast;
    
    public:
      Compiler() { }
    
      std::string file;
    
      void error (const nanoc::location& l, const std::string& m);
      void error (const std::string& m);
    
      bool parseString(const std::string& string);
      bool parse(const std::string& filename);
    
      void setAST(ASTNode* node) { ast = std::unique_ptr<ASTNode>(node); }
    
      void pruneAST();
    
      void printAST();
  };
  
}

#endif