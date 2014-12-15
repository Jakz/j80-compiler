%{
	#include "nanoc.tab.hpp"
  //#include <cassert>
  #include "opcodes.h"
  #include <string>
  
  extern "C"
	{
    extern int yylex(void);
	}
  
  static std::string buffer;
%}

%x sstring

%option noyywrap
%option yylineno

%%

":" { return T_COLON; }
"," { return T_COMMA; }
"'" { return T_QUOTE; }
"(" { return T_LPAREN; }
")" { return T_RPAREN; }
"[" { return T_LBRACK; }
"]" { return T_RBRACK; }

"0x"[a-fA-F0-9]+ { nclval.us = strtol( &yytext[2], NULL, 16); return U16; }
[a-fA-F0-9]+"H" { nclval.us = strtol( yytext, NULL, 16); return U16; }
[a-fA-F0-9]+"h" { nclval.us = strtol( yytext, NULL, 16); return U16; }
"0b"[0-1]+ { nclval.us = strtol( &yytext[2], NULL, 2); return U16; }
[0-1]+"B" { nclval.us = strtol( yytext, NULL, 2); return U16; }
[0-1]+"b" { nclval.us = strtol( yytext, NULL, 2); return U16; }
[+\-]?[0-9]+ { nclval.us = atoi(yytext); return U16;}
"'"[ -~]"'" { nclval.us = yytext[1]; return U16; }

\" { buffer.clear(); BEGIN(sstring); }

<sstring>\" { BEGIN(INITIAL); nclval.str = strdup(buffer.c_str()); return STRING_LITERAL; }
<sstring>\\n { buffer += '\n'; }
<sstring>\\0 { buffer += '\0'; printf(">>>>>>>> ANTANI!\n"); }
<sstring>\\(.|\n) { buffer += yytext[1]; }
<sstring>[^\\\n\"]+ { char *ptr = yytext; while (*ptr) buffer += *ptr++; }

#[^\\\n]+\n { }

[a-zA-Z_][a-zA-Z0-9_]* { nclval.str = strdup(yytext); return STRING; }



\n|"\r\n" { return T_EOL; }
[ \t] { /* ignore whitespace */ }
. { printf("Mystery character %c\n", *yytext); }

%%

// \"(\\.|[^\\"])*\" { yylval.str = strdup(&yytext[1]); yylval.str[strlen(yytext)-2] = 0; return STRING_LITERAL; }
// <sstring>\\[a-fA-F0-9]{1,2} { buffer += strtol(&yytext[1], NULL, 16); }

