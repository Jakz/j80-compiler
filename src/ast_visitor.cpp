#include "ast_visitor.h"

#include "ast.h"

#include <iostream>
#include <iomanip>

using namespace nanoc;
using namespace std;

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

