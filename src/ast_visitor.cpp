#include "ast_visitor.h"

#include "ast.h"

#include <iostream>
#include <iomanip>

using namespace nanoc;
using namespace std;

#pragma mark Generic Visitor

#define DISPATCH(__CLASS_NAME__) { __CLASS_NAME__* inode = dynamic_cast<__CLASS_NAME__*>(node); if (inode) { visit(inode); return; } }
#define OPTIONAL_DISPATCH(__METHOD__) { auto* n = __METHOD__; if (n) n->accept(this); }
#define VISITOR_FUNCTIONALITY_IMPL(__CLASS_NAME__) void Visitor::enteringNode(__CLASS_NAME__* node) { commonEnteringNode(node); }\
void Visitor::exitingNode(__CLASS_NAME__* node) { commonExitingNode(node); }

void Visitor::visit(ASTNode* node)
{
  DISPATCH(ASTList<ASTDeclaration>)
  DISPATCH(ASTList<ASTStatement>)
  DISPATCH(ASTList<ASTExpression>)
  DISPATCH(ASTList<ASTConditionalBlock>)
  DISPATCH(ASTFuncDeclaration)
  DISPATCH(ASTScope)
  DISPATCH(ASTCall)
  DISPATCH(ASTUnaryExpression)
  DISPATCH(ASTBinaryExpression)
  DISPATCH(ASTTernaryExpression)
  DISPATCH(ASTAssign)
  DISPATCH(ASTDeclarationValue<Type::BOOL>)
  DISPATCH(ASTDeclarationValue<Type::BYTE>)
  DISPATCH(ASTDeclarationValue<Type::WORD>)
  DISPATCH(ASTDeclarationArray<Type::BOOL>)
  DISPATCH(ASTDeclarationArray<Type::BYTE>)
  DISPATCH(ASTDeclarationArray<Type::WORD>)
  DISPATCH(ASTWhile)
  DISPATCH(ASTNumber)
  DISPATCH(ASTBool)
  DISPATCH(ASTArrayReference)
  DISPATCH(ASTReference)
  DISPATCH(ASTConditional)
  DISPATCH(ASTElseBlock)
  DISPATCH(ASTIfBlock)
  DISPATCH(ASTReturn)
  
  string error = fmt::sprintf("visit unhandled on %s", Utils::execute(std::string("c++filt ")+typeid(node).name()).c_str());
  cout << error;
  assert(false);
}

VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTDeclaration>)
VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTStatement>)
VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTExpression>)
VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTConditionalBlock>)
VISITOR_FUNCTIONALITY_IMPL(ASTFuncDeclaration)
VISITOR_FUNCTIONALITY_IMPL(ASTScope)
VISITOR_FUNCTIONALITY_IMPL(ASTCall)
VISITOR_FUNCTIONALITY_IMPL(ASTUnaryExpression)
VISITOR_FUNCTIONALITY_IMPL(ASTBinaryExpression)
VISITOR_FUNCTIONALITY_IMPL(ASTTernaryExpression)
VISITOR_FUNCTIONALITY_IMPL(ASTAssign)
VISITOR_FUNCTIONALITY_IMPL(ASTDeclarationValue<Type::BOOL>)
VISITOR_FUNCTIONALITY_IMPL(ASTDeclarationValue<Type::BYTE>)
VISITOR_FUNCTIONALITY_IMPL(ASTDeclarationValue<Type::WORD>)
VISITOR_FUNCTIONALITY_IMPL(ASTDeclarationArray<Type::BOOL>)
VISITOR_FUNCTIONALITY_IMPL(ASTDeclarationArray<Type::BYTE>)
VISITOR_FUNCTIONALITY_IMPL(ASTDeclarationArray<Type::WORD>)
VISITOR_FUNCTIONALITY_IMPL(ASTWhile)
VISITOR_FUNCTIONALITY_IMPL(ASTNumber)
VISITOR_FUNCTIONALITY_IMPL(ASTBool)
VISITOR_FUNCTIONALITY_IMPL(ASTArrayReference)
VISITOR_FUNCTIONALITY_IMPL(ASTReference)
VISITOR_FUNCTIONALITY_IMPL(ASTConditional)
VISITOR_FUNCTIONALITY_IMPL(ASTElseBlock)
VISITOR_FUNCTIONALITY_IMPL(ASTIfBlock)
VISITOR_FUNCTIONALITY_IMPL(ASTReturn)


void Visitor::leafVisit(ASTNode *node)
{
  commonVisit(node);
  commonEnteringNode(node);
  commonExitingNode(node);
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

void Visitor::visit(ASTArrayReference* node)
{
  commonVisit(node);
  enteringNode(node);
  
  node->getIndex()->accept(this);
  
  exitingNode(node);
}

void Visitor::visit(ASTScope* node)
{
  commonVisit(node);
  enteringNode(node);
  
  node->getStatements()->accept(this);
  
  exitingNode(node);
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

void Visitor::visit(ASTDeclarationArray<Type::BOOL>* node)
{
  commonVisit(node);
  enteringNode(node);
  
  OPTIONAL_DISPATCH(node->getInitializer())
  
  exitingNode(node);
}

void Visitor::visit(ASTDeclarationArray<Type::WORD>* node)
{
  commonVisit(node);
  enteringNode(node);
  
  OPTIONAL_DISPATCH(node->getInitializer())
  
  exitingNode(node);
}

void Visitor::visit(ASTDeclarationArray<Type::BYTE>* node)
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

  node->getStatements()->accept(this);
  
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

void PrinterVisitor::commonEnteringNode(ASTNode *node)
{
  //cout << "entering node" << endl;
  ++indent;
}

void PrinterVisitor::commonExitingNode(ASTNode *node)
{
  //cout << "exiting node" << endl;
  --indent;
}

void PrinterVisitor::commonVisit(ASTNode *node)
{
  cout << string(indent*2, ' ') << node->mnemonic() << endl;
}

