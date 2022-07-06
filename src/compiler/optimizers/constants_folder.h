#pragma once

#include "ast.h"
#include "ast_visitor.h"

namespace nanoc::optimizer
{
  class ConstantFolderVisitor : public Visitor
  {
    size_t counter;

  public:
    ConstantFolderVisitor() : counter(0) { }
    
    bool hasFoldedAny() { return counter > 0; }

    ASTNode* exitingNode(ASTBinaryExpression* node) override;
  };

  ASTNode* ConstantFolderVisitor::exitingNode(ASTBinaryExpression* node)
  {
    auto& o1 = node->getOperand1();
    auto& o2 = node->getOperand2();
    auto op = node->getOperation();

    ASTNode* folded = nullptr;

    if (o1->isConstexpr() && o2->isConstexpr())
    {
      switch (op)
      {
        case Binary::ADDITION: 
        {
          folded = new ASTNumber(node->getLocation(), o1->getValue() + o2->getValue());
          break;
        }

        /* if two constants are equal we can fold them to the result */
        case Binary::EQ:
        {
          bool equal = o1->getValue() == o2->getValue();
          folded = new ASTBool(node->getLocation(), equal);
        }
      }
    }

    if (folded)
      ++counter;

    return folded;
  }
}