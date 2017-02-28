%{
	#include "j80parser.hpp"
  #include "j80lexer.h"
  #include "opcodes.h"
  #include "assembler.h"
  #include <string>
  
  using token = Assembler::Parser::token;
  
  #define yyterminate() Assembler::Parser::make_END(loc);


//
  
  static std::string buffer;
  
  static Assembler::location loc;
  
  using namespace Assembler;
%}

%x sstring

%option nodefault
%option noyywrap
%option c++
%option yyclass="Lexer"
%option prefix="J80"


%{
  #define YY_USER_ACTION  loc.columns((int)yyleng);
%}


%%

%{
  loc.step();
%}

":" { return Parser::make_COLON(loc); }
"," { return Parser::make_COMMA(loc); }
"'" { return Parser::make_QUOTE(loc); }
"(" { return Parser::make_LPAREN(loc); }
")" { return Parser::make_RPAREN(loc); }
"[" { return Parser::make_LBRACK(loc); }
"]" { return Parser::make_RBRACK(loc); }

(?i:"a") { return Parser::make_REG8(REG_A, loc); }
(?i:"b") { return Parser::make_REG8(REG_B, loc); }
(?i:"x") { return Parser::make_REG8(REG_X, loc); }
(?i:"y") { return Parser::make_REG8(REG_Y, loc); }
(?i:"c") { return Parser::make_REG8(REG_C, loc); }
(?i:"d") { return Parser::make_REG8(REG_D, loc); }
(?i:"e") { return Parser::make_REG8(REG_E, loc); }
(?i:"f") { return Parser::make_REG8(REG_F, loc); }

(?i:"ba") { return Parser::make_REG16(REG_BA, loc); }
(?i:"xy") { return Parser::make_REG16(REG_XY, loc); }
(?i:"sp") { return Parser::make_REG16(REG_SP, loc); }
(?i:"fp") { return Parser::make_REG16(REG_FP, loc); }
(?i:"cd") { return Parser::make_REG16(REG_CD, loc); }
(?i:"ef") { return Parser::make_REG16(REG_EF, loc); }
(?i:"ix") { return Parser::make_REG16(REG_IX, loc); }
(?i:"iy") { return Parser::make_REG16(REG_IY, loc); }

(?i:"add") { return Parser::make_ALU(ALU_ADD8, loc); }
(?i:"adc") { return Parser::make_ALU(ALU_ADC8, loc); }
(?i:"sub") { return Parser::make_ALU(ALU_SUB8, loc); }
(?i:"sbc") { return Parser::make_ALU(ALU_SBC8, loc); }
(?i:"and") { return Parser::make_ALU(ALU_AND8, loc); }
(?i:"xor") { return Parser::make_ALU(ALU_XOR8, loc); }
(?i:"or") { return Parser::make_ALU(ALU_OR8, loc); }
(?i:"not") { return Parser::make_ALU(ALU_NOT8, loc); }

(?i:"jmp") { return Parser::make_JMP(COND_UNCOND, loc); }
(?i:"jmpc") { return Parser::make_JMP(COND_CARRY, loc); }
(?i:"jmpnc") { return Parser::make_JMP(COND_NCARRY, loc); }
(?i:"jmpz") { return Parser::make_JMP(COND_ZERO, loc); }
(?i:"jmpnz") { return Parser::make_JMP(COND_NZERO, loc); }
(?i:"jmpv") { return Parser::make_JMP(COND_OVERFLOW, loc); }
(?i:"jmpnv") { return Parser::make_JMP(COND_NOVERFLOW, loc); }
(?i:"jmpn") { return Parser::make_JMP(COND_SIGN, loc); }
(?i:"jmpnn") { return Parser::make_JMP(COND_NSIGN, loc); }

(?i:"call") { return Parser::make_CALL(COND_UNCOND, loc); }
(?i:"callc") { return Parser::make_CALL(COND_CARRY, loc); }
(?i:"callnc") { return Parser::make_CALL(COND_NCARRY, loc); }
(?i:"callz") { return Parser::make_CALL(COND_ZERO, loc); }
(?i:"callnz") { return Parser::make_CALL(COND_NZERO, loc); }
(?i:"callv") { return Parser::make_CALL(COND_OVERFLOW, loc); }
(?i:"callnv") { return Parser::make_CALL(COND_NOVERFLOW, loc); }
(?i:"calln") { return Parser::make_CALL(COND_SIGN, loc); }
(?i:"callnn") { return Parser::make_CALL(COND_NSIGN, loc); }

(?i:"ret") { return Parser::make_RET(COND_UNCOND, loc); }
(?i:"retc") { return Parser::make_RET(COND_CARRY, loc); }
(?i:"retnc") { return Parser::make_RET(COND_NCARRY, loc); }
(?i:"retz") { return Parser::make_RET(COND_ZERO, loc); }
(?i:"retnz") { return Parser::make_RET(COND_NZERO, loc); }
(?i:"retv") { return Parser::make_RET(COND_OVERFLOW, loc); }
(?i:"retnv") { return Parser::make_RET(COND_NOVERFLOW, loc); }
(?i:"retn") { return Parser::make_RET(COND_SIGN, loc); }
(?i:"retnn") { return Parser::make_RET(COND_NSIGN, loc); }

(?i:"ld") { return Parser::make_LD(loc); }
(?i:"mov") { return Parser::make_LD(loc); }
(?i:"st") { return Parser::make_ST(loc); }
(?i:"store") { return Parser::make_ST(loc); }
(?i:"load") { return Parser::make_LD(loc); }
(?i:"lf") { return Parser::make_LF(loc); }
(?i:"sf") { return Parser::make_SF(loc); }
(?i:"lsh") { return Parser::make_LSH(loc); }
(?i:"rsh") { return Parser::make_RSH(loc); }
(?i:"cmp") { return Parser::make_CMP(loc); }
(?i:"sext") { return Parser::make_SEXT(loc); }

(?i:"push") { return Parser::make_PUSH(loc); }
(?i:"pop") { return Parser::make_POP(loc); }
(?i:"ei") { return Parser::make_EI(loc); }
(?i:"di") { return Parser::make_DI(loc); }
(?i:"nop") { return Parser::make_NOP(loc); }

(?i:"length") { return Parser::make_DATA_LENGTH(loc); }
(?i:".ascii") { return Parser::make_DATA_ASCII(loc); }
(?i:".asciiz") { return Parser::make_DATA_ASCIIZ(loc); }
(?i:".const") { return Parser::make_DATA_CONST(loc); }
(?i:".reserve") { return Parser::make_DATA_RESERVE(loc); }
(?i:".bytes") { return Parser::make_DATA_BYTES(loc); }
(?i:".words") { return Parser::make_DATA_WORDS(loc); }
(?i:".entry") { return Parser::make_ENTRY(loc); }
(?i:".interrupt") { return Parser::make_INTERRUPT(loc); }
(?i:".stackbase") { return Parser::make_STACK_BASE(loc); }

"0x"[a-fA-F0-9]+ { return Parser::make_U16(strtol( &yytext[2], NULL, 16), loc); }
[a-fA-F0-9]+"H" { return Parser::make_U16(strtol( yytext, NULL, 16), loc); }
[a-fA-F0-9]+"h" { return Parser::make_U16(strtol( yytext, NULL, 16), loc); }
"0b"[0-1]+ { return Parser::make_U16(strtol( &yytext[2], NULL, 2), loc); }
[0-1]+"B" { return Parser::make_U16(strtol( yytext, NULL, 2), loc); }
[0-1]+"b" { return Parser::make_U16(strtol( yytext, NULL, 2), loc); }
[+\-]?[0-9]+ { return Parser::make_U16(atoi(yytext), loc);}
"'"[ -~]"'" { return Parser::make_U16(yytext[1], loc); }

\" { buffer.clear(); BEGIN(sstring); }

<sstring>\" { BEGIN(INITIAL); return Parser::make_LITERAL(buffer, loc); }
<sstring>\\n { buffer += '\n'; }
<sstring>\\0 { buffer += '\0'; }
<sstring>\\(.|\n) { buffer += yytext[1]; }
<sstring>[^\\\n\"]+ { char *ptr = yytext; while (*ptr) buffer += *ptr++; }

[a-zA-Z_][a-zA-Z0-9_]* { return Parser::make_STRING(yytext,loc); }

"#"[^\n]*\n{1,1} { loc.lines(1); }

[\n]+ { Parser::make_EOL(loc); loc.lines(yyleng); }
[ \t] { /* ignore whitespace */ }
. { assembler.error(loc, fmt::sprintf("Unrecognized character: %s", yytext)); }

<<EOF>> { return yyterminate(); }

%%

// \"(\\.|[^\\"])*\" { j80lval.str = strdup(&yytext[1]); j80lval.str[strlen(yytext)-2] = 0; return STRING_LITERAL; }
// <sstring>\\[a-fA-F0-9]{1,2} { buffer += strtol(&yytext[1], NULL, 16); }

