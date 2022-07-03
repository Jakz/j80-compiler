%skeleton "lalr1.cc"
%require "3.0.2"

%defines
%define api.namespace {nanoc}
%define parser_class_name {Parser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert true

%code requires {
  #include "ast.h"

  namespace nanoc
  {
    class Lexer;
    class Compiler;
  }
}

%lex-param { nanoc::Lexer &lexer }
%lex-param { nanoc::Compiler &compiler }
%parse-param { nanoc::Lexer &lexer }
%parse-param { nanoc::Compiler &compiler }

%locations
%initial-action
{
  @$.begin.filename = @$.end.filename = &compiler.file;
};

%define parse.trace
%define parse.error verbose

%code top {
  #include "compiler.h"
  #include "nanoclexer.h"
  #include "nanocparser.hpp"
  
  static nanoc::Parser::symbol_type yylex(nanoc::Lexer &scanner, nanoc::Compiler &compiler) {
    return scanner.get_next_token();
  }
  
  using namespace nanoc;
}

%define api.token.prefix {T_}
%token
  END 0 "end of file"
  EOL "\n"
  COLON ":"
  COMMA ","
  SEMICOL ";"
  LPAREN "("
  RPAREN ")"
  QUOTE "\""
  LBRACK "["
  RBRACK "]"
  STAR "*"
  EQUAL "="
  PLUS "+"
  MINUS "-"
  LBRACE "{"
  RBRACE "}"
  BANG "!"
  TILDE "~"
  QUESTION "?"
  AND "&"
  OR "|"
  XOR "^"
  LOR "||"
  LAND "&&"

  ARROW "->"
  DOT "."
  DEREFERENCE "*"
  ADDRESSOF "&"
;

%token <Binary> COMP "comparison";

%token
  IF "if"
  ELSE "else"
  ELSEIF "elseif"
  WHILE "while"
  FOR "for"
  RETURN "return"
  THEN "then"
  FUNCTION "function"
  ENUM "enum"
  STRUCT "struct"
;

%token <bool> BOOL_VALUE
  TRUE "true"
  FALSE "false"
;

%token
  VOID
  BYTE
  WORD
  BOOL
;

%token <std::string>
  IDENTIFIER "identifier"
  LITERAL "string literal"
;

%token<Value> NUMBER "numeric constant"

%token
  UMINUS
  NO_ELSE
;




%type<ASTDeclaration*> declaration
%type<std::list<ASTDeclaration*>> declarations

%type<ASTFuncDeclaration*> function_declaration
%type<ASTDeclaration*> array_declaration
%type<ASTDeclarationValue*> variable_declaration variable_declaration_initialized
%type<ASTExpression*> expression

%type<ASTEnumDeclaration*> enum_declaration
%type<std::list<ASTEnumEntry*>> enum_entry_list

%type<ASTStructDeclaration*> struct_declaration
%type<std::list<ASTStructField*>> struct_field_list

%type<ASTStatement*> statement
%type<ASTScope*> scope
%type<std::list<ASTStatement*>> statements

%type<std::list<ASTConditionalBlock*>> if_statement

%type<ASTLeftHand*> left_hand

%type<RealType*> real_type
%type<BaseType*> return_type

%type<std::list<ASTExpression*>> expression_list optional_expression_list
%type<std::list<Argument>> type_list optional_type_list

%printer { for (const auto& x : $$) yyoutput << x.name; } <std::list<Argument>>
%printer { yyoutput << ""; } <std::list<ASTStatement*>>
%printer { yyoutput << ""; } <std::list<ASTExpression*>>
%printer { yyoutput << ""; } <std::list<ASTDeclaration*>>
%printer { yyoutput << ""; } <std::list<ASTEnumEntry*>>
%printer { yyoutput << ""; } <std::list<ASTConditionalBlock*>>
%printer { yyoutput << ""; } <std::list<ASTStructField*>>


%nonassoc ELSE


%printer { yyoutput << $$; } <*>

%right QUESTION
%nonassoc BANG
%left AND
%left XOR
%left OR
%left LOR
%left LAND
%left COMP
%left PLUS MINUS
%right UMINUS

%%

start:
  declarations {
    compiler.setAST(new ASTList<ASTDeclaration>(@1, $1));
    compiler.printAST();
  }
;

declarations:
  /* empty */ { $$ = std::list<ASTDeclaration*>(); }
| declarations declaration { $1.push_back($2); $$ = $1; }
;

declaration:
  variable_declaration { $$ = $1; }
  | variable_declaration_initialized { $$ = $1; }
  | array_declaration { $$ = $1; }
  | FUNCTION function_declaration { $$ = $2; }
  | enum_declaration { $$ = $1; }
  | struct_declaration { $$ = $1; }
;

variable_declaration:
  real_type IDENTIFIER SEMICOL
  {
    if (dynamic_cast<RealType*>($1))
      $$ = new ASTDeclarationValue(@1, $2,static_cast<RealType*>($1));
    else
    {
      error(@1, "Type specified for variable declaration can't be used."); YYERROR;
    }
  };
;

variable_declaration_initialized:
 real_type IDENTIFIER EQUAL expression SEMICOL
{
  if (dynamic_cast<RealType*>($1))
  $$ = new ASTDeclarationValue(@1, $2,static_cast<RealType*>($1),$4);
  else
  {
    error(@1, "Type specified for variable declaration can't be used."); YYERROR;
  }
}
;

array_declaration:
  real_type LBRACK RBRACK IDENTIFIER EQUAL LBRACE expression_list RBRACE SEMICOL
  {
    if (dynamic_cast<RealType*>($1))
      $$ = new ASTDeclarationArray(@1, $4, new Array(static_cast<RealType*>($1), $7.size()), $7.size(), $7);
    else
    {
      error(@1, "Type specified for array variable declaration can't be used."); YYERROR;
    }
  }
  | real_type LBRACK NUMBER RBRACK IDENTIFIER SEMICOL
  {
    if (dynamic_cast<RealType*>($1))
      $$ = new ASTDeclarationArray(@1, $5, new Array(static_cast<RealType*>($1), $3), $3);
    else
    {
      error(@1, "Type specified for array variable declaration can't be used."); YYERROR;
    }
  }
  | real_type LBRACK RBRACK IDENTIFIER EQUAL LITERAL SEMICOL
  {
    if (!(dynamic_cast<Byte*>($1) || dynamic_cast<Word*>($1)))
    {
      error(@1, "a string literal can be declared just of type byte[] or word[]"); YYERROR;
    }
    
    const std::string& literal = $6;
    std::list<ASTExpression*> initializer;
    
    for (const auto c : literal)
      initializer.push_back(new ASTNumber(@1, c));

    $$ = new ASTDeclarationArray(@1, $4, new Array(static_cast<RealType*>($1), literal.length()), literal.length(), initializer);
  }
;

function_declaration:
  return_type IDENTIFIER LPAREN optional_type_list RPAREN LBRACE statements RBRACE { $$ = new ASTFuncDeclaration(@1, $2, $1, $4, $7); }
;

enum_declaration:
  ENUM IDENTIFIER LBRACE enum_entry_list RBRACE SEMICOL { $$ = new ASTEnumDeclaration(@1, $2, $4); }
;

enum_entry_list:
  IDENTIFIER EQUAL NUMBER { $$ = std::list<ASTEnumEntry*>(); $$.push_front(new ASTEnumEntry(@1, $1, $3)); }
  | IDENTIFIER { $$ = std::list<ASTEnumEntry*>(); $$.push_front(new ASTEnumEntry(@1, $1));  }
  | enum_entry_list COMMA IDENTIFIER EQUAL NUMBER { $1.push_back(new ASTEnumEntry(@1, $3, $5)); $$ = $1; }
  | enum_entry_list COMMA IDENTIFIER { $1.push_back(new ASTEnumEntry(@1, $3)); $$ = $1; }
;

struct_field_list:
  /* empty */ { $$ = std::list<ASTStructField*>(); }
  | struct_field_list variable_declaration { if ($2) { $1.push_back(new ASTStructField(@1, $2)); } $$ = $1; }
;

struct_declaration:
  STRUCT IDENTIFIER LBRACE struct_field_list RBRACE SEMICOL { $$ = new ASTStructDeclaration(@1, $2, $4); }
;

real_type:
  BYTE { $$ = new Byte(); }
  | WORD { $$ = new Word(); }
  | BOOL { $$ = new Bool(); }
  | IDENTIFIER { $$ = new Named($1); }
  | real_type STAR { $$ = new Pointer($1); }
;

return_type:
  real_type { $$ = $1; }
  | VOID { $$ = new Void(); }

optional_type_list:
  /* empty */ { $$ = std::list<Argument>(); }
  | VOID { $$ = std::list<Argument>(); }
  | type_list { $$ = $1; }
;

type_list:
  real_type IDENTIFIER { $$ = std::list<Argument>(); $$.push_front(Argument($2, $1)); }
  | type_list COMMA real_type IDENTIFIER { $1.push_back(Argument($4, $3)); $$ = $1; }
;

statements:
  /* empty */ { $$ = std::list<ASTStatement*>(); }
  | statements statement { if ($2) { $1.push_back($2); } $$ = $1; }
;

statement:
  left_hand EQUAL expression SEMICOL { $$ = new ASTAssign(@1, $1,$3); }
  | WHILE LPAREN expression RPAREN statement { $$ = new ASTWhile(@1, $3, $5); }
  | if_statement { $$ = new ASTConditional(@1, $1); }
  | RETURN expression SEMICOL { $$ = new ASTReturn(@1, $2); }
  | RETURN SEMICOL { $$ = new ASTReturn(@1); }
  | IDENTIFIER LPAREN optional_expression_list RPAREN SEMICOL { $$ = new ASTCall(@1, $1, $3); }
  | variable_declaration { $$ = $1; }
  | array_declaration { $$ = $1; }
  | scope { $$ = $1; }
  | SEMICOL { $$ = nullptr; }
;

if_statement:
  IF LPAREN expression RPAREN statement { $$ = std::list<ASTConditionalBlock*>(); $$.push_front(new ASTIfBlock(@1, $3, $5)); }
  | if_statement ELSEIF LPAREN expression RPAREN statement { $1.push_back(new ASTIfBlock(@1, $4, $6)); $$ = $1; }
  | if_statement ELSE statement { $1.push_back(new ASTElseBlock(@1, $3)); $$ = $1; }

left_hand:
  IDENTIFIER { $$ = new ASTLeftHand(@1, $1); }
;

scope:
  LBRACE statements RBRACE { $$ = new ASTScope(@1, $2); }

expression:
  NUMBER { $$ = new ASTNumber(@1, $1); }
  | BOOL_VALUE { $$ = new ASTBool(@1, $1); }
  | IDENTIFIER { $$ = new ASTReference(@1, $1); }
  | IDENTIFIER LPAREN optional_expression_list RPAREN { $$ = new ASTCall(@1, $1, $3); }
  | expression LBRACK expression RBRACK { $$ = new ASTArrayReference(@1, $1, $3); }
  | LPAREN expression RPAREN { $$ = $2; }
  | expression PLUS expression { $$ = new ASTBinaryExpression(@1, Binary::ADDITION, $1, $3); }
  | expression MINUS expression { $$ = new ASTBinaryExpression(@1, Binary::SUBTRACTION, $1, $3); }
  | expression AND expression { $$ = new ASTBinaryExpression(@1, Binary::AND, $1, $3); }
  | expression OR expression { $$ = new ASTBinaryExpression(@1, Binary::OR, $1, $3); }
  | expression XOR expression { $$ = new ASTBinaryExpression(@1, Binary::XOR, $1, $3); }
  | expression LAND expression { $$ = new ASTBinaryExpression(@1, Binary::LAND, $1, $3); }
  | expression LOR expression { $$ = new ASTBinaryExpression(@1, Binary::LOR, $1, $3); }
  | expression COMP expression { $$ = new ASTBinaryExpression(@1, $2, $1, $3); }
  | expression QUESTION expression COLON expression { $$ = new ASTTernaryExpression(@1, Ternary::ELVIS,$1,$3,$5); }

  | expression DOT IDENTIFIER { $$ = new ASTFieldAccess(@1, $1, $3, false); }
  | expression ARROW IDENTIFIER { $$ = new ASTFieldAccess(@1, $1, $3, true); }
  | DEREFERENCE expression { $$ = new ASTUnaryExpression(@1, Unary::DEREFERENCE, $2); }
  | AND expression { $$ = new ASTUnaryExpression(@1, Unary::ADDRESSOF, $2); }

  | MINUS expression %prec UMINUS { $$ = new ASTUnaryExpression(@1, Unary::NEG, $2); }
  | BANG expression { $$ = new ASTUnaryExpression(@1, Unary::NOT, $2); }
  | TILDE expression { $$ = new ASTUnaryExpression(@1, Unary::FLIP, $2); }

;

optional_expression_list:
  /* empty */ { $$ = std::list<ASTExpression*>(); }
  | expression_list { $$ = $1; }

expression_list:
  expression { $$ = std::list<ASTExpression*>(); $$.push_front($1); }
  | expression_list COMMA expression { $1.push_back($3); $$ = $1; }

%%

void nanoc::Parser::error(const location_type& l, const std::string& m)
{
  compiler.error(l,m);
}
