#ifndef __AST_VISITOR_H__
#define __AST_VISITOR_H__

#include "utils.h"

namespace nanoc
{

  class ASTNode;

  class Visitor
  {
  public:
    virtual void enteringNode(ASTNode* node) = 0;
    virtual void exitingNode(ASTNode* node) = 0;
    virtual void visit(ASTNode* node) = 0;
  };
    
  
  class PrinterVisitor : public Visitor
  {
  private:
    u16 indent = 0;
    
    void pad(u16 indent);
    
  public:
    PrinterVisitor() : indent(0) { }
    void enteringNode(ASTNode* node);
    void exitingNode(ASTNode* node);
    void visit(ASTNode* node);
  };
  
}

#endif