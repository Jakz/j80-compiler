%{
	#include "nanocparser.hpp"
  #include "nanoclexer.h"
  #include "opcodes.h"
  #include <string>
  
  typedef nanoc::Parser::token token;
  
  #define yyterminate() nanoc::Parser::make_END(loc);


//
  
  static std::string buffer;
  
  static nanoc::location loc;
  
  using namespace nanoc;
%}

%x sstring

%option nodefault
%option noyywrap
%option c++
%option yyclass="Lexer"
%option prefix="NanoC"


%{
  # define YY_USER_ACTION  loc.columns(yyleng);
%}


%%

%{
  loc.step ();
%}

":" { return Parser::make_COLON(loc); }
"," { return Parser::make_COMMA(loc); }
";" { return Parser::make_SEMICOL(loc); }
"'" { return Parser::make_QUOTE(loc); }
"(" { return Parser::make_LPAREN(loc); }
")" { return Parser::make_RPAREN(loc); }
"[" { return Parser::make_LBRACK(loc); }
"]" { return Parser::make_RBRACK(loc); }
"*" { return Parser::make_STAR(loc); }

"byte" { return Parser::make_TYPE(Type::BYTE, loc); }
"byte"[\t ]*"*" { return Parser::make_TYPE(Type::BYTE_PTR, loc); }
"word" { return Parser::make_TYPE(Type::WORD, loc); }
"word"[\t ]*"*" { return Parser::make_TYPE(Type::WORD_PTR, loc); }


"0x"[a-fA-F0-9]+ { return Parser::make_U16(strtol( &yytext[2], NULL, 16), loc); }
[a-fA-F0-9]+"H" { return Parser::make_U16(strtol( yytext, NULL, 16), loc); }
[a-fA-F0-9]+"h" { return Parser::make_U16(strtol( yytext, NULL, 16), loc); }
"0b"[0-1]+ { return Parser::make_U16(strtol( &yytext[2], NULL, 2), loc); }
[0-1]+"B" { return Parser::make_U16(strtol( yytext, NULL, 2), loc); }
[0-1]+"b" { return Parser::make_U16(strtol( yytext, NULL, 2), loc); }
[+\-]?[0-9]+ { return Parser::make_U16(atoi(yytext), loc);}
"'"[ -~]"'" { return Parser::make_U16(yytext[1], loc); }

\" { buffer.clear(); BEGIN(sstring); }

<sstring>\" { BEGIN(INITIAL); return Parser::make_LITERAL(buffer.c_str(), loc); }
<sstring>\\n { buffer += '\n'; }
<sstring>\\0 { buffer += '\0'; printf(">>>>>>>> ANTANI!\n"); }
<sstring>\\(.|\n) { buffer += yytext[1]; }
<sstring>[^\\\n\"]+ { char *ptr = yytext; while (*ptr) buffer += *ptr++; }

[a-zA-Z_][a-zA-Z0-9_]* { return Parser::make_IDENTIFIER(yytext,loc); }

"#"[^\n]*\n{1,1} { Parser::make_EOL(loc); loc.lines(yyleng); loc.step(); }


\n|"\r\n" { Parser::make_EOL(loc); loc.lines(yyleng); loc.step();  }
[ \t] { loc.step(); /* ignore whitespace */ }
. { printf("Mystery character %c\n", *yytext); }

<<EOF>> { return yyterminate(); }

%%

// \"(\\.|[^\\"])*\" { j80lval.str = strdup(&yytext[1]); j80lval.str[strlen(yytext)-2] = 0; return STRING_LITERAL; }
// <sstring>\\[a-fA-F0-9]{1,2} { buffer += strtol(&yytext[1], NULL, 16); }

