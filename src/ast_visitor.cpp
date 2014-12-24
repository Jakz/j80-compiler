#include "ast_visitor.h"

#include "ast.h"

#include <iostream>
#include <iomanip>

using namespace nanoc;
using namespace std;

#pragma mark Generic Visitor

#define DISPATCH(__CLASS_NAME__) { __CLASS_NAME__* inode = dynamic_cast<__CLASS_NAME__*>(node); if (inode) { visit(inode); return; } }
#define OPTIONAL_DISPATCH(__METHOD__) { auto* n = __METHOD__; if (n) n->accept(this); }

void Visitor::visit(ASTNode* node)
{
  DISPATCH(ASTList<ASTDeclaration>)
  DISPATCH(ASTList<ASTStatement>)
  DISPATCH(ASTList<ASTExpression>)
  DISPATCH(ASTList<ASTConditionalBlock>)
  DISPATCH(ASTCall)
  DISPATCH(ASTUnaryExpression)
  DISPATCH(ASTBinaryExpression)
  DISPATCH(ASTTernaryExpression)
  DISPATCH(ASTAssign)
  DISPATCH(ASTDeclarationValue<Type::BOOL>)
  DISPATCH(ASTDeclarationValue<Type::BYTE>)
  DISPATCH(ASTDeclarationValue<Type::WORD>)
  DISPATCH(ASTFuncDeclaration)
  DISPATCH(ASTWhile)
  DISPATCH(ASTNumber)
  DISPATCH(ASTBool)
  DISPATCH(ASTReference)
  DISPATCH(ASTConditional)
  DISPATCH(ASTElseBlock)
  DISPATCH(ASTIfBlock)
  DISPATCH(ASTReturn)
  
  string error = fmt::sprintf("visit unhandled on %s", Utils::execute(std::string("c++filt ")+typeid(node).name()).c_str());
  cout << error;
  assert(false);
}

void Visitor::leafVisit(ASTNode *node)
{
  commonVisit(node);
  enteringNode(node);
  exitingNode(node);
}

void Visitor::visit(ASTNumber* node)
{
  leafVisit(node);
}

void Visitor::visit(ASTBool* node)
{
  leafVisit(node);
}

void Visitor::visit(ASTReference* node)
{
  leafVisit(node);
}

void Visitor::visit(ASTList<ASTDeclaration>* node)
{
  commonVisit(node);
  enteringNode(node);
  
  for (const auto& s : node->getElements())
    s->accept(this);
  
  exitingNode(node);
}

void Visitor::visit(ASTList<ASTStatement>* node)
{
  commonVisit(node);
  enteringNode(node);
  
  for (const auto& s : node->getElements())
    s->accept(this);
  
  exitingNode(node);
}

void Visitor::visit(ASTList<ASTExpression>* node)
{
  commonVisit(node);
  enteringNode(node);
  
  for (const auto& s : node->getElements())
    s->accept(this);
  
  exitingNode(node);
}

void Visitor::visit(ASTList<ASTConditionalBlock>* node)
{
  commonVisit(node);
  enteringNode(node);

  for (const auto& s : node->getElements())
    s->accept(this);
  
  exitingNode(node);
}

void Visitor::visit(ASTConditional* node)
{
  commonVisit(node);
  enteringNode(node);
  
  node->getBlocks()->accept(this);
  
  exitingNode(node);
}

void Visitor::visit(ASTIfBlock* node)
{
  commonVisit(node);
  enteringNode(node);
  
  node->getCondition()->accept(this);
  node->getBody()->accept(this);
  
  exitingNode(node);
}

void Visitor::visit(ASTElseBlock* node)
{
  commonVisit(node);
  enteringNode(node);
  
  if (node->getBody())
    node->getBody()->accept(this);
  
  exitingNode(node);
}

void Visitor::visit(ASTReturn* node)
{
  commonVisit(node);
  enteringNode(node);
  
  OPTIONAL_DISPATCH(node->getValue())
  
  exitingNode(node);
}

void Visitor::visit(ASTCall* node)
{
  commonVisit(node);
  enteringNode(node);
  
  node->getArguments()->accept(this);
  
  exitingNode(node);
}

void Visitor::visit(ASTUnaryExpression* node)
{
  commonVisit(node);
  enteringNode(node);
  
  node->getOperand()->accept(this);
  
  exitingNode(node);
}

void Visitor::visit(ASTBinaryExpression* node)
{
  commonVisit(node);
  enteringNode(node);

  node->getOperand1()->accept(this);
  node->getOperand2()->accept(this);

  exitingNode(node);
}

void Visitor::visit(ASTTernaryExpression* node)
{
  commonVisit(node);
  enteringNode(node);
  
  node->getOperand1()->accept(this);
  node->getOperand2()->accept(this);
  node->getOperand3()->accept(this);
  
  exitingNode(node);
}

void Visitor::visit(ASTAssign* node)
{
  commonVisit(node);
  enteringNode(node);

  node->getRightHand()->accept(this);
  
  exitingNode(node);
}

void Visitor::visit(ASTDeclarationValue<Type::BOOL>* node)
{
  commonVisit(node);
  enteringNode(node);

  OPTIONAL_DISPATCH(node->getInitializer())
  
  exitingNode(node);
}

void Visitor::visit(ASTDeclarationValue<Type::WORD>* node)
{
  commonVisit(node);
  enteringNode(node);

  OPTIONAL_DISPATCH(node->getInitializer())
  
  exitingNode(node);
}

void Visitor::visit(ASTDeclarationValue<Type::BYTE>* node)
{
  commonVisit(node);
  enteringNode(node);

  OPTIONAL_DISPATCH(node->getInitializer())
  
  exitingNode(node);
}

void Visitor::visit(ASTFuncDeclaration* node)
{
  commonVisit(node);
  enteringNode(node);

  node->getBody()->accept(this);
  
  exitingNode(node);
}

void Visitor::visit(ASTWhile* node)
{
  commonVisit(node);
  
  enteringNode(node);
  
  node->getCondition()->accept(this);
  node->getBody()->accept(this);
  
  exitingNode(node);
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

void PrinterVisitor::commonVisit(ASTNode *node)
{
  cout << string(indent*2, ' ') << node->mnemonic() << endl;
}

