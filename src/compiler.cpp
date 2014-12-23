#include "compiler.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace nanoc;

bool Compiler::parseString(const std::string &string)
{
  file = "none";
  istringstream stream(string);
  
  nanoc::Lexer lexer = nanoc::Lexer(*this, &stream);
  nanoc::Parser parser(lexer, *this);
  parser.set_debug_level(false);
  int res = parser.parse();
  return res == 0;
}

bool Compiler::parse(const std::string &filename)
{
  file = filename;
  
  bool shouldGenerateTrace = true;
  
  ifstream is;
  is.open(filename);
  
  nanoc::Lexer lexer = nanoc::Lexer(*this, &is);
  nanoc::Parser parser(lexer, *this);
  parser.set_debug_level(shouldGenerateTrace);
  int res = parser.parse();
  return res == 0;
}

void Compiler::error (const nanoc::location& l, const std::string& m)
{
  cerr << "Compiler error at " << file << ":" << l.begin.line << "," << l.begin.column << " : " << m << endl;
}

void Compiler::error (const std::string& m)
{
  cerr << "Compiler error: " << m << endl;
}

void Compiler::pruneAST()
{
  const UniqueNode& node = ast;
  
  ASTListRecur *asRecList = dynamic_cast<ASTListRecur*>(node.get());

  if (asRecList)
  {
    ASTListRecur* rlist = asRecList;
    ASTListSeq *list = new ASTListSeq();
    
    while (rlist)
    {
      UniqueNode item = rlist->stealItem();
      
      if (item.get())
        list->prepend(std::move(item));
      
      rlist = rlist->getNext();
    }
    
    this->ast = UniqueNode(list);
  }
}

void Compiler::printAST()
{
  if (ast.get())
    ast->recursivePrint(0);
}
