%skeleton "lalr1.cc"
%require "3.0.2"

%defines
%define api.namespace {Assembler}
%define parser_class_name {Parser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert true

%code requires {
  #include "opcodes.h"
  #include "instruction.h"
  #include "location.h"

  namespace Assembler
  {
    class Lexer;
    class J80Assembler;
  }
}

%lex-param { Assembler::Lexer &lexer }
%lex-param { Assembler::J80Assembler &assembler }
%parse-param { Assembler::Lexer &lexer }
%parse-param { Assembler::J80Assembler &assembler }

%locations
%define api.location.type { Assembler::location }
%initial-action
{
  @$.begin.filename = @$.end.filename = &assembler.file;
};

%define parse.trace
%define parse.error verbose

%code top {
  #include "assembler.h"
  #include "j80lexer.h"
  #include "j80parser.hpp"
  #include "instruction.h"
  
  static Assembler::Parser::symbol_type yylex(Assembler::Lexer &scanner, Assembler::J80Assembler &assembler) {
    return scanner.get_next_token();
  }
  
  using namespace Assembler;
}

%define api.token.prefix {T_}
%token
  END 0 "end of file"
  EOL "\n"
  COLON ":"
  COMMA ","
  LPAREN "("
  RPAREN ")"
  QUOTE "\""
  RBRACK "[" LBRACK "]"
;

%token <std::string>
  STRING "identifier"
  LITERAL "string literal"
;

%token <Reg>
  REG8
  REG16
;

%token <u16> U16;

%token <Alu> ALU;

%token <JumpCondition>
  JMP
  CALL
  RET
;

%token <u8> INTSTART;


%token
  LD
  ST
  LF
  SF
  LSH
  RSH
  CMP
  PUSH
  POP
  EI
  DI
  SEXT
  NOP
  DATA_ASCII ".ascii"
  DATA_ASCIIZ ".asciiz"
  DATA_CONST ".const"
  DATA_RESERVE ".reserve"
  DATA_BYTES ".bytes"
  DATA_WORDS ".words"
  DATA_LENGTH "LENGTH"
  ENTRY ".entry"
  INTERRUPT ".interrupt"
  STACK_BASE ".stackbase"
;

%type<Value8> value8
%type<Value16> value16
%type<Address> address
%type<std::list<u8>> u8_array
%type<std::list<u16>> u16_array

%printer { yyoutput << $$; } <*>
%printer { yyoutput << ""; } <Reg>
%printer { yyoutput << ""; } <Address>
%printer { yyoutput << ""; } <Value8>
%printer { yyoutput << ""; } <Value16>
%printer { yyoutput << ""; } <std::list<u8>>
%printer { yyoutput << ""; } <std::list<u16>>


%%

start:
  { } instructions { }
;

instructions:
  | instructions instruction { }
  | instructions EOL { }
  /*| instructions label EOL { }*/
;

instruction:
  
  LD REG8 COMMA REG8 { assembler.add(new InstructionLD_LSH_RSH($2, $4, Alu::TRANSFER_A8, false)); }
| LD REG8 COMMA value8 { assembler.add(new InstructionLD_NN($2, $4)); }

| LD REG16 COMMA REG16 { assembler.add(new InstructionLD_LSH_RSH($2, $4, Alu::TRANSFER_A16, true)); }

/* LD R, [NNNN] */
| LD REG8 COMMA LBRACK value16 RBRACK { assembler.add(new InstructionLD_PTR_NNNN($2, $5)); }

/* LD R, [PP+SS] */
| LD REG8 COMMA LBRACK REG16 value8 RBRACK { assembler.add(new InstructionLD_PTR_PP($2, $5, $6)); }
| LD REG8 COMMA LBRACK REG16 RBRACK { assembler.add(new InstructionLD_PTR_PP($2, $5, 0)); }
/* LD P, NNNN */
| LD REG16 COMMA value16 { assembler.add(new InstructionLD_NNNN($2, $4)); }

/* ST [NNNN], R */
| ST LBRACK value16 RBRACK COMMA REG8 { assembler.add(new InstructionST_PTR_NNNN($6, $3)); }
  
/* ST [PP+SS], R */
| ST LBRACK REG16 U16 RBRACK COMMA REG8 { assembler.add(new InstructionST_PTR_PP($7, $3, $4)); }
| ST LBRACK REG16 RBRACK COMMA REG8 { assembler.add(new InstructionST_PTR_PP($6, $3, 0)); }


/* ALU R, S, U */
| ALU REG8 COMMA REG8 COMMA REG8 { assembler.add(new InstructionALU_R($2, $4, $6, $1, false)); }
| ALU REG8 COMMA REG8 { assembler.add(new InstructionALU_R($2, $2, $4, $1, false)); }
| ALU REG8 { assembler.add(new InstructionALU_R($2, $2, $2, $1, false)); }

| ALU REG16 COMMA REG16 COMMA REG16 { assembler.add(new InstructionALU_R($2, $4, $6, $1, true)); }
| ALU REG16 COMMA REG16 { assembler.add(new InstructionALU_R($2, $2, $4, $1, true)); }
| ALU REG16 { assembler.add(new InstructionALU_R($2, $2, $2, $1, true)); }

/* ALU R, NN */
| ALU REG8 COMMA value8 { assembler.add(new InstructionALU_R_NN($2, $2, $1, $4)); }
| ALU REG8 COMMA REG8 COMMA value8 { assembler.add(new InstructionALU_R_NN($2, $4, $1, $6)); }

/* ALU R, NNNN */
| ALU REG16 COMMA value16 { assembler.add(new InstructionALU_NNNN($2, $2, $1 | Alu::EXTENDED_BIT, $4)); }
| ALU REG16 COMMA REG16 COMMA value16 { assembler.add(new InstructionALU_NNNN($2, $4, $1 | Alu::EXTENDED_BIT, $6)); }

| LSH REG8 COMMA REG8 { assembler.add(new InstructionLD_LSH_RSH($2, $4, Alu::LSH8, false)); }
| RSH REG8 COMMA REG8 { assembler.add(new InstructionLD_LSH_RSH((Reg)$2, $4, Alu::RSH8, false)); }
| LSH REG8 { assembler.add(new InstructionLD_LSH_RSH((Reg)$2, $2, Alu::LSH8, false)); }
| RSH REG8 { assembler.add(new InstructionLD_LSH_RSH((Reg)$2, $2, Alu::RSH8, false)); }
| LSH REG16 COMMA REG16 { assembler.add(new InstructionLD_LSH_RSH($2, (Reg)$4, Alu::LSH16, false)); }
| RSH REG16 COMMA REG16 { assembler.add(new InstructionLD_LSH_RSH($2, (Reg)$4, Alu::RSH16, false)); }
| LSH REG16 { assembler.add(new InstructionLD_LSH_RSH($2, $2, Alu::LSH16, false)); }
| RSH REG16 { assembler.add(new InstructionLD_LSH_RSH($2, $2, Alu::RSH16, false)); }

| SEXT REG8 {
  if ($2 != Reg::A && $2 != Reg::D && $2 != Reg::F && $2 != Reg::Y)
  {
    error(@2, fmt::format("SEXT instruction can be executed only on lower regs (A, D, F or Y), {} is not valid.", Opcodes::reg8($2))); YYERROR;
  }

  assembler.add(new InstructionSEXT($2));
}

| JMP address { assembler.add(new InstructionJMP_NNNN($1, $2)); }
| JMP REG16 { assembler.add(new InstructionJMP_PP($1, (Reg)$2)); }
| CALL address { assembler.add(new InstructionCALL_NNNN($1, $2)); }
| RET { assembler.add(new InstructionRET($1)); }

| PUSH REG8 { assembler.add(new InstructionPUSH8($2)); }
| POP REG8 { assembler.add(new InstructionPOP8($2)); }

| PUSH REG16 { assembler.add(new InstructionPUSH16($2)); }
| POP REG16 { assembler.add(new InstructionPOP16($2)); }

| LF REG8 { assembler.add(new InstructionLF($2)); }
| SF REG8 { assembler.add(new InstructionSF($2)); }

| EI { assembler.add(new InstructionEI()); }
| DI { assembler.add(new InstructionDI()); }
| NOP { assembler.add(new InstructionNOP()); }

/* CMP 8 bit */
| CMP REG8 COMMA REG8 { assembler.add(new InstructionCMP_R_S($2, $4, false)); }
| CMP REG8 { assembler.add(new InstructionCMP_NN($2, Value8(0))); }
| CMP REG8 COMMA value8 { assembler.add(new InstructionCMP_NN($2, $4)); }

/* CMP 16 bit */
| CMP REG16 COMMA value16 { assembler.add(new InstructionCMP_NNNN($2, $4)); }
| CMP REG16 { assembler.add(new InstructionCMP_NNNN($2, Value16(0))); }
| CMP REG16 COMMA REG16 { assembler.add(new InstructionCMP_R_S($2, $4, true)); }


| DATA_CONST STRING U16 { assembler.addConstValue($2, $3); }

| DATA_ASCII STRING LITERAL { assembler.addData($2, DataSegmentEntry($3, false)); }
| DATA_ASCIIZ STRING LITERAL { assembler.addData($2, DataSegmentEntry($3, true)); }
| DATA_RESERVE STRING U16 { assembler.addData($2, DataSegmentEntry($3)); }
| DATA_BYTES STRING LBRACK u8_array RBRACK { assembler.addData($2, $4); }
| DATA_WORDS STRING LBRACK u16_array RBRACK { assembler.addData($2, $4); }


| ENTRY U16 { if (!assembler.setEntryPoint($2)) { error(@1, "entry point specified more than once"); YYERROR; } }

| INTERRUPT U16 {
  if ($2 >= assembler.maxNumberOfInterrupts())
  {
    error(@1, "interrupt index specified over maximum allowed index"); YYERROR;
  }
  else if (!assembler.isInterruptAvailable($2))
  {
    error(@1, "interrupt already specified for the index"); YYERROR;
  }
  else
  {
    assembler.markInterrupt($2);
    assembler.add(new InterruptEntryPoint($2));
  }
}

| STACK_BASE U16 {
  if (!assembler.setStackBase($2)) {
    error(@1, "stack base specified more than once"); YYERROR;
  }
  
}

/* label */
| STRING COLON {
  if ($1.size() >= 2 && $1[0] == '_' && $1[1] == '_')
  {
    error(@1, fmt::format("label '{}': names starting with '__' are reserved named", $1));
    YYERROR;
  }
  
  assembler.add(new Label($1));
}

/*  | JUMP STRING EOL { ASSEMBLER->asmJump($2, false); }
  | JUMPC STRING EOL { ASSEMBLER->asmJump($2, true); }*/

;

address:
U16 { $$ = Address($1); }
| STRING { $$ = Address($1); }

value8:
U16 {
  if (!valueFitsType<u8>($1)) {
    error(@1, "8bit value exceeds range: " + std::to_string((s16)$1));
    YYERROR;
  }
  $$ = Value8($1);
}
| DATA_LENGTH LPAREN STRING RPAREN { $$ = Value8(Value8::Type::DATA_LENGTH, $3); }
| STRING { $$ = Value8(Value8::Type::CONST, $1); }

value16:
U16 { $$ = Value16($1); }
| DATA_LENGTH LPAREN STRING RPAREN { $$ = Value16(Value16::Type::DATA_LENGTH, $3); }
| STRING { $$ = Value16(Value16::Type::LABEL_ADDRESS, $1); }
| STRING U16 { $$ = Value16(Value16::Type::LABEL_ADDRESS, $1, $2); }

u8_array:
U16 { $$ = std::list<u8>(); $$.push_front($1); }
| u8_array COMMA U16 { $1.push_back($3); $$ = $1; }

u16_array:
U16 { $$ = std::list<u16>(); $$.push_front($1); }
| u16_array COMMA U16 { $1.push_back($3); $$ = $1; }


/*
label:
  STRING COLON { ASSEMBLER->addLabelHere($1); }
;*/

%%

void Assembler::Parser::error(const location_type& l, const std::string& m)
{
  assembler.error(l,m);
}
