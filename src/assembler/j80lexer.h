#ifndef __J80_SCANNER_H__
#define __J80_SCANNER_H__

#if ! defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer J80FlexLexer
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL Assembler::Parser::symbol_type Assembler::Lexer::get_next_token()

#include "j80parser.hpp"

namespace Assembler
{
  
  class J80Assembler;
  
  class Lexer : public yyFlexLexer
  {
  public:
    
    Lexer(J80Assembler &assembler, std::istream *in) : yyFlexLexer(in), assembler(assembler) {}

    virtual Assembler::Parser::symbol_type get_next_token();
    virtual ~Lexer() { }
    
  private:
    
    J80Assembler &assembler;
  };
  
}

#endif