#include "ast_visitor.h"

#include "ast.h"

#include <iostream>
#include <iomanip>

using namespace nanoc;
using namespace std;

#pragma mark Generic Visitor

#define DISPATCH(__CLASS_NAME__) { if (dynamic_cast<__CLASS_NAME__*>(node)) return visit(dynamic_cast<__CLASS_NAME__*>(node)); }
#define OPTIONAL_DISPATCH(__METHOD__) { auto* n = __METHOD__; if (n) dispatch(n.get()); }
#define VISITOR_FUNCTIONALITY_IMPL(__CLASS_NAME__) void Visitor::enteringNode(__CLASS_NAME__* node) { commonEnteringNode(node); }\
ASTNode* Visitor::exitingNode(__CLASS_NAME__* node) { commonExitingNode(node); return nullptr; }

ASTNode* Visitor::dispatch(ASTNode* node)
{
  DISPATCH(ASTList<ASTDeclaration>)
  DISPATCH(ASTList<ASTStatement>)
  DISPATCH(ASTList<ASTExpression>)
  DISPATCH(ASTList<ASTConditionalBlock>)
  DISPATCH(ASTList<ASTEnumEntry>)
  DISPATCH(ASTList<ASTStructField>)
  DISPATCH(ASTFuncDeclaration)
  DISPATCH(ASTEnumDeclaration)
  DISPATCH(ASTStructDeclaration)
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
  DISPATCH(ASTStructField)
  DISPATCH(ASTFieldAccess)
  DISPATCH(ASTDereference)
  DISPATCH(ASTAddressOf)
  DISPATCH(ASTLeftHand)
  
  string error = fmt::sprintf("visit unhandled on %s", Utils::execute(std::string("c++filt ")+typeid(node).name()).c_str());
  cout << error;
  assert(false);
  return nullptr;
}

VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTDeclaration>)
VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTStatement>)
VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTExpression>)
VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTConditionalBlock>)
VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTEnumEntry>)
VISITOR_FUNCTIONALITY_IMPL(ASTList<ASTStructField>)
VISITOR_FUNCTIONALITY_IMPL(ASTFuncDeclaration)
VISITOR_FUNCTIONALITY_IMPL(ASTEnumDeclaration)
VISITOR_FUNCTIONALITY_IMPL(ASTStructDeclaration)
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
VISITOR_FUNCTIONALITY_IMPL(ASTStructField)
VISITOR_FUNCTIONALITY_IMPL(ASTFieldAccess)
VISITOR_FUNCTIONALITY_IMPL(ASTDereference)
VISITOR_FUNCTIONALITY_IMPL(ASTAddressOf)
VISITOR_FUNCTIONALITY_IMPL(ASTLeftHand)


template<typename T>
void Visitor::dispatchAndReplace(unique_ptr<T>& ptr)
{
  if (ptr)
  {
    ASTNode* node = dispatch(ptr.get());

    T* cnode = dynamic_cast<T*>(node);
    
    if (cnode)
      ptr.reset(cnode);
  }
}

ASTNode* Visitor::visit(ASTNumber* node)
{
  commonVisit(node);
  enteringNode(node);
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTBool* node)
{
  commonVisit(node);
  enteringNode(node);
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTReference* node)
{
  commonVisit(node);
  enteringNode(node);
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTEnumEntry* node)
{
  commonVisit(node);
  enteringNode(node);
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTStructField* node)
{
  commonVisit(node);
  enteringNode(node);
  //dispatchAndReplace(node->getDeclaration());
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTFieldAccess* node)
{
  commonVisit(node);
  enteringNode(node);
  dispatchAndReplace(node->getExpression());
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTDereference* node)
{
  commonVisit(node);
  enteringNode(node);
  dispatchAndReplace(node->getExpression());
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTAddressOf* node)
{
  commonVisit(node);
  enteringNode(node);
  dispatchAndReplace(node->getExpression());
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTArrayReference* node)
{
  commonVisit(node);
  enteringNode(node);

  dispatchAndReplace(node->getIndex());

  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTScope* node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatchAndReplace(node->getStatements());

  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTList<ASTDeclaration>* node)
{
  commonVisit(node);
  enteringNode(node);
  
  for (auto& s : node->getElements())
    dispatchAndReplace(s);
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTList<ASTStatement>* node)
{
  commonVisit(node);
  enteringNode(node);
  
  for (auto& s : node->getElements())
    dispatchAndReplace(s);
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTList<ASTExpression>* node)
{
  commonVisit(node);
  enteringNode(node);
  
  for (auto& s : node->getElements())
    dispatchAndReplace(s);
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTList<ASTConditionalBlock>* node)
{
  commonVisit(node);
  enteringNode(node);

  for (auto& s : node->getElements())
    dispatchAndReplace(s);
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTList<ASTEnumEntry>* node)
{
  commonVisit(node);
  enteringNode(node);
    
  for (auto& s : node->getElements())
    dispatchAndReplace(s);
    
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTList<ASTStructField>* node)
{
  commonVisit(node);
  enteringNode(node);
  
  for (auto& s : node->getElements())
    dispatchAndReplace(s);
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTConditional* node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatchAndReplace(node->getBlocks());
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTIfBlock* node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatchAndReplace(node->getCondition());
  dispatchAndReplace(node->getBody());
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTElseBlock*node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatchAndReplace(node->getBody());
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTReturn* node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatchAndReplace(node->getValue());
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTCall* node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatchAndReplace(node->getArguments());
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTUnaryExpression* node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatchAndReplace(node->getOperand());
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTBinaryExpression* node)
{
  commonVisit(node);
  enteringNode(node);

  dispatchAndReplace(node->getOperand1());
  dispatchAndReplace(node->getOperand2());

  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTTernaryExpression* node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatchAndReplace(node->getOperand1());
  dispatchAndReplace(node->getOperand2());
  dispatchAndReplace(node->getOperand3());
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTAssign* node)
{
  commonVisit(node);
  enteringNode(node);

  dispatchAndReplace(node->getLeftHand());
  dispatchAndReplace(node->getRightHand());
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTLeftHand* node)
{
  commonVisit(node);
  enteringNode(node);
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTDeclarationValue* node)
{
  commonVisit(node);
  enteringNode(node);

  dispatchAndReplace(node->getInitializer());
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTDeclarationArray* node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatchAndReplace(node->getInitializer());
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTFuncDeclaration* node)
{
  commonVisit(node);
  enteringNode(node);

  dispatchAndReplace(node->getStatements());
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTEnumDeclaration* node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatchAndReplace(node->getEntries());
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTStructDeclaration* node)
{
  commonVisit(node);
  enteringNode(node);
  
  dispatchAndReplace(node->getFields());
  
  return exitingNode(node);
}

ASTNode* Visitor::visit(ASTWhile* node)
{
  commonVisit(node);
  
  enteringNode(node);
  
  dispatchAndReplace(node->getCondition());
  dispatchAndReplace(node->getBody());
  
  return exitingNode(node);
}


void PrinterVisitor::pad(u16 indent)
{

}

void PrinterVisitor::commonEnteringNode(ASTNode* node)
{
  //cout << "entering node" << endl;
  ++indent;
}

void PrinterVisitor::commonExitingNode(ASTNode* node)
{
  //cout << "exiting node" << endl;
  --indent;
}

void PrinterVisitor::commonVisit(ASTNode* node)
{
  cout << string(indent*2, ' ') << node->mnemonic() << endl;
}

