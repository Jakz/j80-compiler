/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_J80_SRC_ASSEMBLER_J80PARSER_HPP_INCLUDED
# define YY_J80_SRC_ASSEMBLER_J80PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int j80debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_EOL = 258,
    T_COLON = 259,
    T_COMMA = 260,
    T_LPAREN = 261,
    T_RPAREN = 262,
    T_QUOTE = 263,
    T_RBRACK = 264,
    T_LBRACK = 265,
    TOKEN_LENGTH = 266,
    TOKEN_REG8 = 267,
    TOKEN_REG16 = 268,
    TOKEN_ALU = 269,
    TOKEN_COND = 270,
    TOKEN_NEAR = 271,
    TOKEN_LD = 272,
    TOKEN_JMP = 273,
    TOKEN_ST = 274,
    TOKEN_LF = 275,
    TOKEN_SF = 276,
    TOKEN_LSH = 277,
    TOKEN_RSH = 278,
    TOKEN_CMP = 279,
    TOKEN_CALL = 280,
    TOKEN_RET = 281,
    TOKEN_PUSH = 282,
    TOKEN_POP = 283,
    TOKEN_INT = 284,
    TOKEN_EI = 285,
    TOKEN_DI = 286,
    TOKEN_NOP = 287,
    TOKEN_DATA_ASCII = 288,
    TOKEN_DATA_RESERVE = 289,
    TOKEN_DATA_RAW = 290,
    TOKEN_INTERRUPT = 291,
    TOKEN_INTERRUPT_END = 292,
    U16 = 293,
    STRING = 294,
    STRING_LITERAL = 295
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 42 "src/assembler/j80.ypp" /* yacc.c:1915  */

	unsigned short us;
  signed short ss;
	char *str;

#line 101 "./src/assembler/j80parser.hpp" /* yacc.c:1915  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE j80lval;

int j80parse (void);

#endif /* !YY_J80_SRC_ASSEMBLER_J80PARSER_HPP_INCLUDED  */
