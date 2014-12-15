%{
	#include "j80.tab.hpp"
  //#include <cassert>
  #include "opcodes.h"
  #include <string>
  
  extern "C"
	{
    extern int yylex(void);
	}
  
  std::string buffer;
%}

%x sstring

%option noyywrap
%option yylineno

%%

":" { return T_COLON; }
"," { return T_COMMA; }
"'" { return T_QUOTE; }

"A"|"a" { yylval.us = REG_A; return TOKEN_REG8; }
"B"|"b" { yylval.us = REG_B; return TOKEN_REG8; }
"X"|"x" { yylval.us = REG_X; return TOKEN_REG8; }
"Y"|"y" { yylval.us = REG_Y; return TOKEN_REG8; }
"C"|"s" { yylval.us = REG_C; return TOKEN_REG8; }
"D"|"t" { yylval.us = REG_D; return TOKEN_REG8; }
"E"|"p" { yylval.us = REG_E; return TOKEN_REG8; }
"F"|"c" { yylval.us = REG_F; return TOKEN_REG8; }

"BA"|"ba" { yylval.us = REG_BA; return TOKEN_REG16; }
"XY"|"xy" { yylval.us = REG_XY; return TOKEN_REG16; }
"SP"|"sp" { yylval.us = REG_SP; return TOKEN_REG16; }
"FP"|"fp" { yylval.us = REG_FP; return TOKEN_REG16; }
"CD"|"cd" { yylval.us = REG_CD; return TOKEN_REG16; }
"EF"|"ef" { yylval.us = REG_EF; return TOKEN_REG16; }
"IX"|"ix" { yylval.us = REG_IX; return TOKEN_REG16; }
"IY"|"iy" { yylval.us = REG_IY; return TOKEN_REG16; }

"ADD"|"add" { yylval.us = ALU_ADD8; return TOKEN_ALU; }
"ADC"|"adc" { yylval.us = ALU_ADC8; return TOKEN_ALU; }
"SUB"|"sub" { yylval.us = ALU_SUB8; return TOKEN_ALU; }
"SBC"|"sbc" { yylval.us = ALU_SBC8; return TOKEN_ALU; }
"AND"|"and" { yylval.us = ALU_AND8; return TOKEN_ALU; }
"XOR"|"xor" { yylval.us = ALU_XOR8; return TOKEN_ALU; }
"OR"|"or" { yylval.us = ALU_OR8; return TOKEN_ALU; }
"NOT"|"not" { yylval.us = ALU_NOT8; return TOKEN_ALU; }

"JMP"|"jmp" { yylval.us = COND_UNCOND; return TOKEN_JMP; }
"JMPC"|"jmpc" { yylval.us = COND_CARRY; return TOKEN_JMP; }
"JMPNC"|"jmpnc" { yylval.us = COND_NCARRY; return TOKEN_JMP; }
"JMPZ"|"jmpz" { yylval.us = COND_ZERO; return TOKEN_JMP; }
"JMPNZ"|"jmpnz" { yylval.us = COND_NZERO; return TOKEN_JMP; }
"JMPV"|"jmpv" { yylval.us = COND_OVERFLOW; return TOKEN_JMP; }
"JMPNV"|"jmpnv" { yylval.us = COND_NOVERFLOW; return TOKEN_JMP; }
"JMPN"|"jmpn" { yylval.us = COND_SIGN; return TOKEN_JMP; }
"JMPNN"|"jmpnn" { yylval.us = COND_NSIGN; return TOKEN_JMP; }

"CALL"|"jmp" { yylval.us = COND_UNCOND; return TOKEN_CALL; }
"CALLC"|"jmpc" { yylval.us = COND_CARRY; return TOKEN_CALL; }
"CALLNC"|"jmpnc" { yylval.us = COND_NCARRY; return TOKEN_CALL; }
"CALLZ"|"jmpz" { yylval.us = COND_ZERO; return TOKEN_CALL; }
"CALLNZ"|"jmpnz" { yylval.us = COND_NZERO; return TOKEN_CALL; }
"CALLV"|"jmpv" { yylval.us = COND_OVERFLOW; return TOKEN_CALL; }
"CALLNV"|"jmpnv" { yylval.us = COND_NOVERFLOW; return TOKEN_CALL; }
"CALLN"|"jmpn" { yylval.us = COND_SIGN; return TOKEN_CALL; }
"CALLNN"|"jmpnn" { yylval.us = COND_NSIGN; return TOKEN_CALL; }

"RET"|"jmp" { yylval.us = COND_UNCOND; return TOKEN_RET; }
"RETC"|"jmpc" { yylval.us = COND_CARRY; return TOKEN_RET; }
"RETNC"|"jmpnc" { yylval.us = COND_NCARRY; return TOKEN_RET; }
"RETZ"|"jmpz" { yylval.us = COND_ZERO; return TOKEN_RET; }
"RETNZ"|"jmpnz" { yylval.us = COND_NZERO; return TOKEN_RET; }
"RETV"|"jmpv" { yylval.us = COND_OVERFLOW; return TOKEN_RET; }
"RETNV"|"jmpnv" { yylval.us = COND_NOVERFLOW; return TOKEN_RET; }
"RETN"|"jmpn" { yylval.us = COND_SIGN; return TOKEN_RET; }
"RETNN"|"jmpnn" { yylval.us = COND_NSIGN; return TOKEN_RET; }

"IRQ0"|"irq0" { yylval.us = 0; return TOKEN_INTERRUPT; }
"IRQ1"|"irq1" { yylval.us = 1; return TOKEN_INTERRUPT; }
"IRQ2"|"irq2" { yylval.us = 2; return TOKEN_INTERRUPT; }
"IRQ3"|"irq3" { yylval.us = 3; return TOKEN_INTERRUPT; }
"IRQEND"|"irqend" { return TOKEN_INTERRUPT_END; }

"NEAR"|"near" { return TOKEN_NEAR; }

"LD"|"ld" { return TOKEN_LD; }
"MOV"|"mov" { return TOKEN_LD; }
"ST"|"st" { return TOKEN_ST; }
"STORE"|"store" { return TOKEN_ST; }
"LOAD"|"load" { return TOKEN_LD; }
"LF"|"lf" { return TOKEN_LF; }
"SF"|"sf" { return TOKEN_SF; }
"LSH"|"lsh" { return TOKEN_LSH; }
"RSH"|"rsh" { return TOKEN_RSH; }
"CMP"|"cmp" { return TOKEN_CMP; }
"(" { return T_LPAREN; }
")" { return T_RPAREN; }
"[" { return T_LBRACK; }
"]" { return T_RBRACK; }

"PUSH"|"push" { return TOKEN_PUSH; }
"POP"|"pop" { return TOKEN_POP; }
"EI"|"ei" { return TOKEN_EI; }
"DI"|"di" { return TOKEN_DI; }
"INT"|"int" { return TOKEN_INT; }
"NOP"|"nop" { return TOKEN_NOP; }

"LENGTH"|"length" { return TOKEN_LENGTH; }
".ASCII"|".ascii" { return TOKEN_DATA_ASCII; }
".RESERVE"|".reserve" { return TOKEN_DATA_RESERVE; }
".RAW"|".raw" { return TOKEN_DATA_RAW; }


"0x"[a-fA-F0-9]+ { yylval.us = strtol( &yytext[2], NULL, 16); return U16; }
[a-fA-F0-9]+"H" { yylval.us = strtol( yytext, NULL, 16); return U16; }
[a-fA-F0-9]+"h" { yylval.us = strtol( yytext, NULL, 16); return U16; }
"0b"[0-1]+ { yylval.us = strtol( &yytext[2], NULL, 2); return U16; }
[0-1]+"B" { yylval.us = strtol( yytext, NULL, 2); return U16; }
[0-1]+"b" { yylval.us = strtol( yytext, NULL, 2); return U16; }
[+\-]?[0-9]+ { yylval.us = atoi(yytext); return U16;}
"'"[ -~]"'" { yylval.us = yytext[1]; return U16; }

\" { buffer.clear(); BEGIN(sstring); }

<sstring>\" { BEGIN(INITIAL); yylval.str = strdup(buffer.c_str()); return STRING_LITERAL; }
<sstring>\\n { buffer += '\n'; }
<sstring>\\0 { buffer += '\0'; printf(">>>>>>>> ANTANI!\n"); }
<sstring>\\(.|\n) { buffer += yytext[1]; }
<sstring>[^\\\n\"]+ { char *ptr = yytext; while (*ptr) buffer += *ptr++; }

#[^\\\n]+\n { }

[a-zA-Z_][a-zA-Z0-9_]* { yylval.str = strdup(yytext); return STRING; }



\n|"\r\n" { return T_EOL; }
[ \t] { /* ignore whitespace */ }
. { printf("Mystery character %c\n", *yytext); }

%%

// \"(\\.|[^\\"])*\" { yylval.str = strdup(&yytext[1]); yylval.str[strlen(yytext)-2] = 0; return STRING_LITERAL; }
// <sstring>\\[a-fA-F0-9]{1,2} { buffer += strtol(&yytext[1], NULL, 16); }

