#include "ast_visitor.h"

#include "ast.h"

#include <iostream>
#include <iomanip>

using namespace nanoc;
using namespace std;

#pragma mark Generic Visitor

void Visitor::visit(ASTList<ASTStatement>* node)
{
  for (const auto& s : node->getStatements())
    s->accept(this);
}

void Visitor::visit(ASTList<ASTExpression>* node)
{
  for (const auto& s : node->getStatements())
    s->accept(this);
}

void Visitor::visit(ASTCall* node)
{
  node->getArguments()->accept(this);
}

void Visitor::visit(ASTUnaryExpression* node)
{
  node->getOperand()->accept(this);
}

void Visitor::visit(ASTBinaryExpression* node)
{
  node->getOperand1()->accept(this);
  node->getOperand2()->accept(this);
}

void Visitor::visit(ASTAssign* node)
{
  node->getRightHand()->accept(this);
}

void Visitor::visit(ASTDeclarationValue<Type::BOOL>* node)
{
  node->getInitializer()->accept(this);
}

void Visitor::visit(ASTDeclarationValue<Type::WORD>* node)
{
  node->getInitializer()->accept(this);
}

void Visitor::visit(ASTDeclarationValue<Type::BYTE>* node)
{
  node->getInitializer()->accept(this);
}

void Visitor::visit(ASTFuncDeclaration* node)
{
  node->getBody()->accept(this);
}

void Visitor::visit(ASTWhile* node)
{
  node->getCondition()->accept(this);
  node->getBody()->accept(this);
}


void PrinterVisitor::pad(u16 indent)
{

}

void PrinterVisitor::enteringNode(ASTNode *node)
{
  //cout << "entering node" << endl;
  ++indent;
}

void PrinterVisitor::exitingNode(ASTNode *node)
{
  //cout << "exiting node" << endl;
  --indent;
}

void PrinterVisitor::visit(ASTNode *node)
{
  cout << string(indent*2, ' ') << node->mnemonic() << endl;
}

