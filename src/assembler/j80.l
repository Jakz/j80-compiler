%{
	#include "j80parser.hpp"
  #include "j80lexer.h"
  #include "opcodes.h"
  #include "assembler.h"
  #include <string>
  
  typedef Assembler::Parser::token token;
  
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
  # define YY_USER_ACTION  loc.columns((int)yyleng);
%}


%%

%{
  loc.step();
%}

":" { return Parser::make_COLON(loc); }
"," { return Parser::make_COMMA(loc); }
"'" { return Parser::make_QUOTE(loc); }

"A"|"a" { return Parser::make_REG8(REG_A, loc); }
"B"|"b" { return Parser::make_REG8(REG_B, loc); }
"X"|"x" { return Parser::make_REG8(REG_X, loc); }
"Y"|"y" { return Parser::make_REG8(REG_Y, loc); }
"C"|"C" { return Parser::make_REG8(REG_C, loc); }
"D"|"D" { return Parser::make_REG8(REG_D, loc); }
"E"|"E" { return Parser::make_REG8(REG_E, loc); }
"F"|"F" { return Parser::make_REG8(REG_F, loc); }

"BA"|"ba" { return Parser::make_REG16(REG_BA, loc); }
"XY"|"xy" { return Parser::make_REG16(REG_XY, loc); }
"SP"|"sp" { return Parser::make_REG16(REG_SP, loc); }
"FP"|"fp" { return Parser::make_REG16(REG_FP, loc); }
"CD"|"cd" { return Parser::make_REG16(REG_CD, loc); }
"EF"|"ef" { return Parser::make_REG16(REG_EF, loc); }
"IX"|"ix" { return Parser::make_REG16(REG_IX, loc); }
"IY"|"iy" { return Parser::make_REG16(REG_IY, loc); }

"ADD"|"add" { return Parser::make_ALU(ALU_ADD8, loc); }
"ADC"|"adc" { return Parser::make_ALU(ALU_ADC8, loc); }
"SUB"|"sub" { return Parser::make_ALU(ALU_SUB8, loc); }
"SBC"|"sbc" { return Parser::make_ALU(ALU_SBC8, loc); }
"AND"|"and" { return Parser::make_ALU(ALU_AND8, loc); }
"XOR"|"xor" { return Parser::make_ALU(ALU_XOR8, loc); }
"OR"|"or" { return Parser::make_ALU(ALU_OR8, loc); }
"NOT"|"not" { return Parser::make_ALU(ALU_NOT8, loc); }

"JMP"|"jmp" { return Parser::make_JMP(COND_UNCOND, loc); }
"JMPC"|"jmpc" { return Parser::make_JMP(COND_CARRY, loc); }
"JMPNC"|"jmpnc" { return Parser::make_JMP(COND_NCARRY, loc); }
"JMPZ"|"jmpz" { return Parser::make_JMP(COND_ZERO, loc); }
"JMPNZ"|"jmpnz" { return Parser::make_JMP(COND_NZERO, loc); }
"JMPV"|"jmpv" { return Parser::make_JMP(COND_OVERFLOW, loc); }
"JMPNV"|"jmpnv" { return Parser::make_JMP(COND_NOVERFLOW, loc); }
"JMPN"|"jmpn" { return Parser::make_JMP(COND_SIGN, loc); }
"JMPNN"|"jmpnn" { return Parser::make_JMP(COND_NSIGN, loc); }

"CALL"|"jmp" { return Parser::make_CALL(COND_UNCOND, loc); }
"CALLC"|"jmpc" { return Parser::make_CALL(COND_CARRY, loc); }
"CALLNC"|"jmpnc" { return Parser::make_CALL(COND_NCARRY, loc); }
"CALLZ"|"jmpz" { return Parser::make_CALL(COND_ZERO, loc); }
"CALLNZ"|"jmpnz" { return Parser::make_CALL(COND_NZERO, loc); }
"CALLV"|"jmpv" { return Parser::make_CALL(COND_OVERFLOW, loc); }
"CALLNV"|"jmpnv" { return Parser::make_CALL(COND_NOVERFLOW, loc); }
"CALLN"|"jmpn" { return Parser::make_CALL(COND_SIGN, loc); }
"CALLNN"|"jmpnn" { return Parser::make_CALL(COND_NSIGN, loc); }

"RET"|"jmp" { return Parser::make_RET(COND_UNCOND, loc); }
"RETC"|"jmpc" { return Parser::make_RET(COND_CARRY, loc); }
"RETNC"|"jmpnc" { return Parser::make_RET(COND_NCARRY, loc); }
"RETZ"|"jmpz" { return Parser::make_RET(COND_ZERO, loc); }
"RETNZ"|"jmpnz" { return Parser::make_RET(COND_NZERO, loc); }
"RETV"|"jmpv" { return Parser::make_RET(COND_OVERFLOW, loc); }
"RETNV"|"jmpnv" { return Parser::make_RET(COND_NOVERFLOW, loc); }
"RETN"|"jmpn" { return Parser::make_RET(COND_SIGN, loc); }
"RETNN"|"jmpnn" { return Parser::make_RET(COND_NSIGN, loc); }

"LD"|"ld" { return Parser::make_LD(loc); }
"MOV"|"mov" { return Parser::make_LD(loc); }
"ST"|"st" { return Parser::make_ST(loc); }
"STORE"|"store" { return Parser::make_ST(loc); }
"LOAD"|"load" { return Parser::make_LD(loc); }
"LF"|"lf" { return Parser::make_LF(loc); }
"SF"|"sf" { return Parser::make_SF(loc); }
"LSH"|"lsh" { return Parser::make_LSH(loc); }
"RSH"|"rsh" { return Parser::make_RSH(loc); }
"CMP"|"cmp" { return Parser::make_CMP(loc); }
"(" { return Parser::make_LPAREN(loc); }
")" { return Parser::make_RPAREN(loc); }
"[" { return Parser::make_LBRACK(loc); }
"]" { return Parser::make_RBRACK(loc); }

"SEXT" { return Parser::make_SEXT(loc); }

"PUSH"|"push" { return Parser::make_PUSH(loc); }
"POP"|"pop" { return Parser::make_POP(loc); }
"EI"|"ei" { return Parser::make_EI(loc); }
"DI"|"di" { return Parser::make_DI(loc); }
"NOP"|"nop" { return Parser::make_NOP(loc); }

"LENGTH"|"length" { return Parser::make_DATA_LENGTH(loc); }
".ASCII"|".ascii" { return Parser::make_DATA_ASCII(loc); }
".CONST"|".const" { return Parser::make_DATA_CONST(loc); }
".RESERVE"|".reserve" { return Parser::make_DATA_RESERVE(loc); }
".RAW"|".raw" { return Parser::make_DATA_RAW(loc); }
".ENTRY"|".entry" { return Parser::make_ENTRY(loc); }
".INTERRUPT"|".interrupt" { return Parser::make_INTERRUPT(loc); }
".STACKBASE"|".stackbase" { return Parser::make_STACK_BASE(loc); }

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

[a-zA-Z_][a-zA-Z0-9_]* { return Parser::make_STRING(yytext,loc); }

"#"[^\n]*\n{1,1} { Parser::make_EOL(loc); loc.lines((int)yyleng); }


\n|"\r\n" { Parser::make_EOL(loc); loc.lines((int)yyleng); }
[ \t] { loc.step(); /* ignore whitespace */ }
. { assembler.error(loc, "Unrecognized character."); }

<<EOF>> { return yyterminate(); }

%%

// \"(\\.|[^\\"])*\" { j80lval.str = strdup(&yytext[1]); j80lval.str[strlen(yytext)-2] = 0; return STRING_LITERAL; }
// <sstring>\\[a-fA-F0-9]{1,2} { buffer += strtol(&yytext[1], NULL, 16); }

