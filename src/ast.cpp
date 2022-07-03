#include "ast.h"

using namespace nanoc;


const Type* ASTTernaryExpression::getType(const SymbolTable& table) const
{
  if (op == Ternary::ELVIS)
    return operand1->getType(table);
  
  return nullptr;
}


const Type* ASTBinaryExpression::getType(const SymbolTable& table) const
{
  switch (op) {
    case Binary::ADDITION:
    case Binary::SUBTRACTION:
      
    case Binary::AND:
    case Binary::XOR:
    case Binary::OR:
      return operand1->getType(table);
      
    case Binary::EQ:
    case Binary::NEQ:
    case Binary::GREATER:
    case Binary::GREATEREQ:
    case Binary::LESS:
    case Binary::LESSEQ:
    case Binary::LAND:
    case Binary::LOR:
      return new Bool();

    default:
      return nullptr;
  }
}


const Type* ASTUnaryExpression::getType(const SymbolTable& table) const
{
  switch (op) {
    case Unary::ADDRESSOF: return new Pointer(operand->getType(table));
    case Unary::DEREFERENCE: return static_cast<const Pointer*>(operand->getType(table))->innerType();
    case Unary::NOT: return new Bool();
      
    case Unary::NEG:
    case Unary::FLIP:
      return operand->getType(table);
      
    default:
      return nullptr;

  }
}

const Type* ASTFieldAccess::getType(const SymbolTable& table) const
{
  
  
  return nullptr;
}