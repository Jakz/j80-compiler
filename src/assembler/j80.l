%{
	#include "j80parser.hpp"
  //#include <cassert>
  #include "opcodes.h"
  #include <string>
  
  extern "C"
	{
    extern int j80lex(void);
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

"A"|"a" { j80lval.us = REG_A; return TOKEN_REG8; }
"B"|"b" { j80lval.us = REG_B; return TOKEN_REG8; }
"X"|"x" { j80lval.us = REG_X; return TOKEN_REG8; }
"Y"|"y" { j80lval.us = REG_Y; return TOKEN_REG8; }
"C"|"s" { j80lval.us = REG_C; return TOKEN_REG8; }
"D"|"t" { j80lval.us = REG_D; return TOKEN_REG8; }
"E"|"p" { j80lval.us = REG_E; return TOKEN_REG8; }
"F"|"c" { j80lval.us = REG_F; return TOKEN_REG8; }

"BA"|"ba" { j80lval.us = REG_BA; return TOKEN_REG16; }
"XY"|"xy" { j80lval.us = REG_XY; return TOKEN_REG16; }
"SP"|"sp" { j80lval.us = REG_SP; return TOKEN_REG16; }
"FP"|"fp" { j80lval.us = REG_FP; return TOKEN_REG16; }
"CD"|"cd" { j80lval.us = REG_CD; return TOKEN_REG16; }
"EF"|"ef" { j80lval.us = REG_EF; return TOKEN_REG16; }
"IX"|"ix" { j80lval.us = REG_IX; return TOKEN_REG16; }
"IY"|"iy" { j80lval.us = REG_IY; return TOKEN_REG16; }

"ADD"|"add" { j80lval.us = ALU_ADD8; return TOKEN_ALU; }
"ADC"|"adc" { j80lval.us = ALU_ADC8; return TOKEN_ALU; }
"SUB"|"sub" { j80lval.us = ALU_SUB8; return TOKEN_ALU; }
"SBC"|"sbc" { j80lval.us = ALU_SBC8; return TOKEN_ALU; }
"AND"|"and" { j80lval.us = ALU_AND8; return TOKEN_ALU; }
"XOR"|"xor" { j80lval.us = ALU_XOR8; return TOKEN_ALU; }
"OR"|"or" { j80lval.us = ALU_OR8; return TOKEN_ALU; }
"NOT"|"not" { j80lval.us = ALU_NOT8; return TOKEN_ALU; }

"JMP"|"jmp" { j80lval.us = COND_UNCOND; return TOKEN_JMP; }
"JMPC"|"jmpc" { j80lval.us = COND_CARRY; return TOKEN_JMP; }
"JMPNC"|"jmpnc" { j80lval.us = COND_NCARRY; return TOKEN_JMP; }
"JMPZ"|"jmpz" { j80lval.us = COND_ZERO; return TOKEN_JMP; }
"JMPNZ"|"jmpnz" { j80lval.us = COND_NZERO; return TOKEN_JMP; }
"JMPV"|"jmpv" { j80lval.us = COND_OVERFLOW; return TOKEN_JMP; }
"JMPNV"|"jmpnv" { j80lval.us = COND_NOVERFLOW; return TOKEN_JMP; }
"JMPN"|"jmpn" { j80lval.us = COND_SIGN; return TOKEN_JMP; }
"JMPNN"|"jmpnn" { j80lval.us = COND_NSIGN; return TOKEN_JMP; }

"CALL"|"jmp" { j80lval.us = COND_UNCOND; return TOKEN_CALL; }
"CALLC"|"jmpc" { j80lval.us = COND_CARRY; return TOKEN_CALL; }
"CALLNC"|"jmpnc" { j80lval.us = COND_NCARRY; return TOKEN_CALL; }
"CALLZ"|"jmpz" { j80lval.us = COND_ZERO; return TOKEN_CALL; }
"CALLNZ"|"jmpnz" { j80lval.us = COND_NZERO; return TOKEN_CALL; }
"CALLV"|"jmpv" { j80lval.us = COND_OVERFLOW; return TOKEN_CALL; }
"CALLNV"|"jmpnv" { j80lval.us = COND_NOVERFLOW; return TOKEN_CALL; }
"CALLN"|"jmpn" { j80lval.us = COND_SIGN; return TOKEN_CALL; }
"CALLNN"|"jmpnn" { j80lval.us = COND_NSIGN; return TOKEN_CALL; }

"RET"|"jmp" { j80lval.us = COND_UNCOND; return TOKEN_RET; }
"RETC"|"jmpc" { j80lval.us = COND_CARRY; return TOKEN_RET; }
"RETNC"|"jmpnc" { j80lval.us = COND_NCARRY; return TOKEN_RET; }
"RETZ"|"jmpz" { j80lval.us = COND_ZERO; return TOKEN_RET; }
"RETNZ"|"jmpnz" { j80lval.us = COND_NZERO; return TOKEN_RET; }
"RETV"|"jmpv" { j80lval.us = COND_OVERFLOW; return TOKEN_RET; }
"RETNV"|"jmpnv" { j80lval.us = COND_NOVERFLOW; return TOKEN_RET; }
"RETN"|"jmpn" { j80lval.us = COND_SIGN; return TOKEN_RET; }
"RETNN"|"jmpnn" { j80lval.us = COND_NSIGN; return TOKEN_RET; }

"IRQ0"|"irq0" { j80lval.us = 0; return TOKEN_INTERRUPT; }
"IRQ1"|"irq1" { j80lval.us = 1; return TOKEN_INTERRUPT; }
"IRQ2"|"irq2" { j80lval.us = 2; return TOKEN_INTERRUPT; }
"IRQ3"|"irq3" { j80lval.us = 3; return TOKEN_INTERRUPT; }
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


"0x"[a-fA-F0-9]+ { j80lval.us = strtol( &yytext[2], NULL, 16); return U16; }
[a-fA-F0-9]+"H" { j80lval.us = strtol( yytext, NULL, 16); return U16; }
[a-fA-F0-9]+"h" { j80lval.us = strtol( yytext, NULL, 16); return U16; }
"0b"[0-1]+ { j80lval.us = strtol( &yytext[2], NULL, 2); return U16; }
[0-1]+"B" { j80lval.us = strtol( yytext, NULL, 2); return U16; }
[0-1]+"b" { j80lval.us = strtol( yytext, NULL, 2); return U16; }
[+\-]?[0-9]+ { j80lval.us = atoi(yytext); return U16;}
"'"[ -~]"'" { j80lval.us = yytext[1]; return U16; }

\" { buffer.clear(); BEGIN(sstring); }

<sstring>\" { BEGIN(INITIAL); j80lval.str = strdup(buffer.c_str()); return STRING_LITERAL; }
<sstring>\\n { buffer += '\n'; }
<sstring>\\0 { buffer += '\0'; printf(">>>>>>>> ANTANI!\n"); }
<sstring>\\(.|\n) { buffer += yytext[1]; }
<sstring>[^\\\n\"]+ { char *ptr = yytext; while (*ptr) buffer += *ptr++; }

#[^\\\n]+\n { }

[a-zA-Z_][a-zA-Z0-9_]* { j80lval.str = strdup(yytext); return STRING; }



\n|"\r\n" { return T_EOL; }
[ \t] { /* ignore whitespace */ }
. { printf("Mystery character %c\n", *yytext); }

%%

// \"(\\.|[^\\"])*\" { j80lval.str = strdup(&yytext[1]); j80lval.str[strlen(yytext)-2] = 0; return STRING_LITERAL; }
// <sstring>\\[a-fA-F0-9]{1,2} { buffer += strtol(&yytext[1], NULL, 16); }

