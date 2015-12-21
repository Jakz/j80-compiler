%{
	#include "nanocparser.hpp"
  #include "nanoclexer.h"
  #include "opcodes.h"
  #include "compiler.h"
  #include <string>
  
  typedef nanoc::Parser::token token;
  
  #define yyterminate() nanoc::Parser::make_END(loc);


//
  
  static std::string buffer;
  
  static nanoc::location loc;
  
  using namespace nanoc;
%}

%x sstring
%x scomment

%option nodefault
%option noyywrap
%option c++
%option yyclass="Lexer"
%option prefix="NanoC"


%{
  # define YY_USER_ACTION  loc.columns((int)yyleng);
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
"=" { return Parser::make_EQUAL(loc); }
"{" { return Parser::make_LBRACE(loc); }
"}" { return Parser::make_RBRACE(loc); }
"+" { return Parser::make_PLUS(loc); }
"-" { return Parser::make_MINUS(loc); }
"!" { return Parser::make_BANG(loc); }
"~" { return Parser::make_TILDE(loc); }
"?" { return Parser::make_QUESTION(loc); }
"&" { return Parser::make_AND(loc); }
"|" { return Parser::make_OR(loc); }
"^" { return Parser::make_XOR(loc); }
"||" { return Parser::make_LOR(loc); }
"&&" { return Parser::make_LAND(loc); }

"->" { return Parser::make_ARROW(loc); }
"*" { return Parser::make_DEREFERENCE(loc); }
"." { return Parser::make_DOT(loc); }
"&" { return Parser::make_ADDRESSOF(loc); }

"!=" { return Parser::make_COMP(Binary::NEQ, loc); }
"==" { return Parser::make_COMP(Binary::EQ, loc); }
">=" { return Parser::make_COMP(Binary::GREATEREQ, loc); }
"<=" { return Parser::make_COMP(Binary::LESSEQ, loc); }
">" { return Parser::make_COMP(Binary::GREATER, loc); }
"<" { return Parser::make_COMP(Binary::LESS, loc); }


"void" { return Parser::make_VOID(loc);}
"bool" { return Parser::make_BOOL(loc); }
"byte" { return Parser::make_BYTE(loc); }
"word" { return Parser::make_WORD(loc); }

"while" { return Parser::make_WHILE(loc); }
"if" { return Parser::make_IF(loc); }
"else" { return Parser::make_ELSE(loc); }
"for" { return Parser::make_FOR(loc); }
"return" { return Parser::make_RETURN(loc); }
"then" { return Parser::make_THEN(loc); }
"elseif" { return Parser::make_ELSEIF(loc); }
"function" { return Parser::make_FUNCTION(loc); }
"enum" { return Parser::make_ENUM(loc); }
"struct" { return Parser::make_STRUCT(loc); }

"true" { return Parser::make_BOOL_VALUE(true, loc); }
"false" { return Parser::make_BOOL_VALUE(false, loc); }


"0x"[a-fA-F0-9]+ { return Parser::make_NUMBER(strtol( &yytext[2], NULL, 16), loc); }
[a-fA-F0-9]+"H" { return Parser::make_NUMBER(strtol( yytext, NULL, 16), loc); }
[a-fA-F0-9]+"h" { return Parser::make_NUMBER(strtol( yytext, NULL, 16), loc); }
"0b"[0-1]+ { return Parser::make_NUMBER(strtol( &yytext[2], NULL, 2), loc); }
[0-1]+"B" { return Parser::make_NUMBER(strtol( yytext, NULL, 2), loc); }
[0-1]+"b" { return Parser::make_NUMBER(strtol( yytext, NULL, 2), loc); }
[+\-]?[0-9]+ { return Parser::make_NUMBER(atoi(yytext), loc);}
"'"[ -~]"'" { return Parser::make_NUMBER(yytext[1], loc); }

<INITIAL>\" { buffer.clear(); BEGIN(sstring); }

<sstring>\" { BEGIN(INITIAL); return Parser::make_LITERAL(buffer.c_str(), loc); }
<sstring>\\n { buffer += '\n'; }
<sstring>\\0 { buffer += '\0'; printf(">>>>>>>> ANTANI!\n"); }
<sstring>\\(.|\n) { buffer += yytext[1]; }
<sstring>[^\\\n\"]+ { char *ptr = yytext; while (*ptr) buffer += *ptr++; }

<INITIAL>"/*" { BEGIN(scomment); }
<scomment>"*/" { BEGIN(INITIAL); }
<scomment>\n|"\r\n" { loc.lines(yyleng); loc.step();  }
<scomment>.+ { }


[a-zA-Z_][a-zA-Z0-9_]* { return Parser::make_IDENTIFIER(yytext,loc); }

"//"[^\n]*\n { Parser::make_EOL(loc); loc.lines(1); loc.step(); }


\n|"\r\n" { Parser::make_EOL(loc); loc.lines(yyleng); loc.step();  }
[ \t] { loc.step(); /* ignore whitespace */ }
. { compiler.error(loc, "Unrecognized character."); }

<<EOF>> { return yyterminate(); }

%%

// \"(\\.|[^\\"])*\" { j80lval.str = strdup(&yytext[1]); j80lval.str[strlen(yytext)-2] = 0; return STRING_LITERAL; }
// <sstring>\\[a-fA-F0-9]{1,2} { buffer += strtol(&yytext[1], NULL, 16); }

