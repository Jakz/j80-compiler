#include "ast_visitor.h"

#include "ast.h"

#include <iostream>
#include <iomanip>

using namespace nanoc;
using namespace std;

#pragma mark Generic Visitor

#define DISPATCH(__CLASS_NAME__) { if (dynamic_cast<__CLASS_NAME__*>(node.get())) { visit(dynamic_cast<unique_ptr<__CLASS_NAME__>&>(node)); return; } }
#define OPTIONAL_DISPATCH(__METHOD__) { auto* n = __METHOD__; if (n) dispatch(n); }
#define VISITOR_FUNCTIONALITY_IMPL(__CLASS_NAME__) void Visitor::enteringNode(std::unique_ptr<__CLASS_NAME__>& node) { commonEnteringNode(node.get()); }\
void Visitor::exitingNode(std::unique_ptr<__CLASS_NAME__>& node) { commonExitingNode(node.get()); }

void Visitor::dispatch(unique_ptr<ASTNode>& node)
{
  DISPATCH(ASTList<ASTDeclaration>)
  DISPATCH(ASTList<ASTStatement>)
  DISPATCH(ASTList<ASTExpression>)
  DISPATCH(ASTList<ASTConditionalBlock>)
  DISPATCH(ASTList<ASTEnumEntry>)
  DISPATCH(ASTFuncDeclaration)
  DISPATCH(ASTEnumDeclaration)
  DISPATCH(ASTScope)
  DISPATCH(ASTCall)
  DISPATCH(ASTUnaryExpression)
  DISPATCH(ASTBinaryExpression)
  DISPATCH(ASTTernaryExpression)
  DISPATCH(ASTAssign)
  DISPATCH(ASTDeclarationValue)
  DISPATCH(ASTDeclarationArray)
  DISPATCH(ASTWhile)
  DISPATCH(ASTNumber)
  DISPATCH(ASTBool)
  DISPATCH(ASTArrayReference)
  DISPATCH(ASTReference)
  DISPATCH(ASTConditional)
  DISPATCH(ASTElseBlock)
  DISPATCH(ASTIfBlock)
  DISPATCH(ASTReturn)
  DISPATCH(ASTEnumEntry)
  
  string error = fmt::sprintf("visit unhandled on %s", Utils::execute(std::string("c++filt ")+typeid(node).name()).c_str());
  cout << error;
  assert(false);
}

VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTDeclaration>)
VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTStatement>)
VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTExpression>)
VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTConditionalBlock>)
VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTEnumEntry>)
VISITOR_FUNCTIONALITY_IMPL(ASTFuncDeclaration)
VISITOR_FUNCTIONALITY_IMPL(ASTEnumDeclaration)
VISITOR_FUNCTIONALITY_IMPL(ASTScope)
VISITOR_FUNCTIONALITY_IMPL(ASTCall)
VISITOR_FUNCTIONALITY_IMPL(ASTUnaryExpression)
VISITOR_FUNCTIONALITY_IMPL(ASTBinaryExpression)
VISITOR_FUNCTIONALITY_IMPL(ASTTernaryExpression)
VISITOR_FUNCTIONALITY_IMPL(ASTAssign)
VISITOR_FUNCTIONALITY_IMPL(ASTDeclarationValue)
VISITOR_FUNCTIONALITY_IMPL(ASTDeclarationArray)
VISITOR_FUNCTIONALITY_IMPL(ASTWhile)
VISITOR_FUNCTIONALITY_IMPL(ASTNumber)
VISITOR_FUNCTIONALITY_IMPL(ASTBool)
VISITOR_FUNCTIONALITY_IMPL(ASTArrayReference)
VISITOR_FUNCTIONALITY_IMPL(ASTReference)
VISITOR_FUNCTIONALITY_IMPL(ASTConditional)
VISITOR_FUNCTIONALITY_IMPL(ASTElseBlock)
VISITOR_FUNCTIONALITY_IMPL(ASTIfBlock)
VISITOR_FUNCTIONALITY_IMPL(ASTReturn)
VISITOR_FUNCTIONALITY_IMPL(ASTEnumEntry)

void Visitor::visit(unique_ptr<ASTNumber>& node)
{
  commonVisit(node);
  enteringNode(node);
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTBool>& node)
{
  commonVisit(node);
  enteringNode(node);
  exitingNode(node);}

void Visitor::visit(unique_ptr<ASTReference>& node)
{
  commonVisit(node);
  enteringNode(node);
  exitingNode(node);}

void Visitor::visit(unique_ptr<ASTEnumEntry>& node)
{
  commonVisit(node);
  enteringNode(node);
  exitingNode(node);}

void Visitor::visit(unique_ptr<ASTArrayReference>& node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatch(node->getIndex());
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTScope>& node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatch(node->getStatements());
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTList<ASTDeclaration>>& node)
{
  commonVisit(node);
  enteringNode(node);
  
  for (const auto& s : node->getElements())
    dispatch(s);
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTList<ASTStatement>>& node)
{
  commonVisit(node);
  enteringNode(node);
  
  for (const auto& s : node->getElements())
    dispatch(s);
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTList<ASTExpression>>& node)
{
  commonVisit(node);
  enteringNode(node);
  
  for (const auto& s : node->getElements())
    dispatch(s);
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTList<ASTConditionalBlock>>& node)
{
  commonVisit(node);
  enteringNode(node);

  for (const auto& s : node->getElements())
    dispatch(s);
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTList<ASTEnumEntry>>& node)
{
  commonVisit(node);
  enteringNode(node);
    
  for (const auto& s : node->getElements())
    dispatch(s);
    
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTConditional>& node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatch(node->getBlocks());
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTIfBlock>& node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatch(node->getCondition());
  dispatch(node->getBody());
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTElseBlock>& node)
{
  commonVisit(node);
  enteringNode(node);
  
  OPTIONAL_DISPATCH(node->getBody())
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTReturn>& node)
{
  commonVisit(node);
  enteringNode(node);
  
  OPTIONAL_DISPATCH(node->getValue())
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTCall>& node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatch(node->getArguments());
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTUnaryExpression>& node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatch(node->getOperand();
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTBinaryExpression>& node)
{
  commonVisit(node);
  enteringNode(node);

  dispatch(node->getOperand1());
  dispatch(node->getOperand2());

  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTTernaryExpression>& node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatch(node->getOperand1());
  dispatch(node->getOperand2());
  dispatch(node->getOperand3());
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTAssign>& node)
{
  commonVisit(node);
  enteringNode(node);

  dispatch(node->getRightHand());
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTDeclarationValue>& node)
{
  commonVisit(node);
  enteringNode(node);

  OPTIONAL_DISPATCH(node->getInitializer())
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTDeclarationArray>& node)
{
  commonVisit(node);
  enteringNode(node);
  
  OPTIONAL_DISPATCH(node->getInitializer())
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTFuncDeclaration>& node)
{
  commonVisit(node);
  enteringNode(node);

  dispatch(node->getStatements());
  
  exitingNode(node);
}

void Visitor::visit(unique_ptr<ASTEnumDeclaration>& node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatch(node->getEntries());
  
  exitingNode(node);
}


void Visitor::visit(unique_ptr<ASTWhile>& node)
{
  commonVisit(node);
  
  enteringNode(node);
  
  dispatch(node->getCondition());
  dispatch(node->getBody());
  
  exitingNode(node);
}


void PrinterVisitor::pad(u16 indent)
{

}

void PrinterVisitor::commonEnteringNode(unique_ptr<ASTNode> node)
{
  //cout << "entering node" << endl;
  ++indent;
}

void PrinterVisitor::commonExitingNode(unique_ptr<ASTNode> node)
{
  //cout << "exiting node" << endl;
  --indent;
}

void PrinterVisitor::commonVisit(unique_ptr<ASTNode> node)
{
  cout << string(indent*2, ' ') << node->mnemonic() << endl;
}

