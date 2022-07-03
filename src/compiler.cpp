#include "compiler.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "ast_visitor.h"

using namespace std;
using namespace nanoc;

bool Compiler::parseString(const std::string &string)
{
  file = "none";
  istringstream stream(string);
  
  nanoc::Lexer lexer(*this, &stream);
  nanoc::Parser parser(lexer, *this);
  parser.set_debug_level(false);
  int res = parser.parse();
  return res == 0;
}

bool Compiler::parse(const std::string &filename)
{
  file = filename;
  
  bool shouldGenerateTrace = false;
  
  ifstream is;
  is.open(filename);
  
  nanoc::Lexer lexer(*this, &is);
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

void Compiler::printAST()
{
  if (ast.get())
  {
    //unique_ptr<PrinterVisitor> visitor = unique_ptr<PrinterVisitor>(new PrinterVisitor());

  }
  
  SymbolsVisitor svisitor;
  
  try
  {
    svisitor.dispatch(ast.get());
    svisitor.getTable().print();
    
    TypeCheckVisitor tvisitor = TypeCheckVisitor(svisitor.getTable());
    tvisitor.dispatch(ast.get());
    
    EnumReplaceVisitor evisitor = EnumReplaceVisitor(svisitor.getTable());
    evisitor.dispatch(ast.get());
    
    PrinterVisitor visitor;
    visitor.dispatch(ast.get());
  }
  catch (const compiler_exception& exception)
  {
    cout << "Error: " << exception.what() << endl;
  }
  
  

  
    //ast->recursivePrint(0);
}
