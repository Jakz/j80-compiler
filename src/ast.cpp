#include "ast.h"

using namespace nanoc;


std::string Mnemonics::mnemonicForBinary(Binary op)
{
  switch (op) {
    case Binary::ADDITION: return "+";
    case Binary::SUBTRACTION: return "-";
    case Binary::AND: return "&";
    case Binary::OR: return "|";
    case Binary::XOR: return "^";
    case Binary::EQ: return "==";
    case Binary::NEQ: return "!=";
    case Binary::GREATEREQ: return ">=";
    case Binary::LESSEQ: return "<=";
    case Binary::GREATER: return ">";
    case Binary::LESS: return "<";
    default: return "";
  }
}

std::string Mnemonics::mnemonicForUnary(Unary op)
{
  switch (op) {
    case Unary::NOT: return "!";
    case Unary::INCR: return "++";
    case Unary::DECR: return "--";
    case Unary::NEG: return "-";
    default: return "";
  }
}

std::string Mnemonics::mnemonicForType(Type type, u16 param)
{
  switch (type) {
    case Type::BOOL: return "bool";
    case Type::BYTE: return "byte";
    case Type::WORD: return "word";
    case Type::BOOL_PTR: return "bool*";
    case Type::BYTE_PTR: return "byte*";
    case Type::WORD_PTR: return "word*";
    case Type::BYTE_ARRAY: return "byte[]";
    case Type::WORD_ARRAY: return "word[]";
    case Type::VOID: return "void";
    default: return "invalid";
  }
}