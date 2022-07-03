#ifndef __NANOC_SCANNER_H__
#define __NANOC_SCANNER_H__

#if ! defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer NanoCFlexLexer
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL nanoc::Parser::symbol_type nanoc::Lexer::get_next_token()

#include "nanocparser.hpp"

namespace nanoc
{
  
  class Compiler;
  
  class Lexer : public yyFlexLexer
  {
  public:
    
    Lexer(Compiler &compiler, std::istream *in) : yyFlexLexer(in), compiler(compiler) {}

    virtual nanoc::Parser::symbol_type get_next_token();
    virtual ~Lexer() { }
    
  private:
    
    Compiler &compiler;
  };
  
}

#endif