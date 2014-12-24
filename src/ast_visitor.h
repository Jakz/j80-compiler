#ifndef __AST_VISITOR_H__
#define __AST_VISITOR_H__

#include "utils.h"

namespace nanoc
{
  template<typename T>
  class ASTList;
  
  template<Type T>
  class ASTDeclarationValue;
  
  class ASTFuncDeclaration;
  
  class ASTNode;
  
  class ASTStatement;
  class ASTNumber;
  class ASTBool;
  class ASTExpression;
  class ASTUnaryExpression;
  class ASTBinaryExpression;
  class ASTReference;
  class ASTCall;
  class ASTWhile;
  
  class ASTAssign;

  class Visitor
  {
  public:
    virtual void enteringNode(ASTNode* node) = 0;
    virtual void exitingNode(ASTNode* node) = 0;
    //virtual void visit(ASTNode* node) = 0;
    
    virtual void visit(ASTList<ASTStatement>* node);
    virtual void visit(ASTList<ASTExpression>* node);
    virtual void visit(ASTNumber* node) { }
    virtual void visit(ASTBool* node) { }
    virtual void visit(ASTReference* node) { }
    virtual void visit(ASTCall* node);
    virtual void visit(ASTBinaryExpression* node);
    virtual void visit(ASTUnaryExpression* node);
    virtual void visit(ASTAssign* node);
    
    virtual void visit(ASTDeclarationValue<Type::BOOL>* node);
    virtual void visit(ASTDeclarationValue<Type::WORD>* node);
    virtual void visit(ASTDeclarationValue<Type::BYTE>* node);
    
    virtual void visit(ASTFuncDeclaration* node);
    
    virtual void visit(ASTWhile* node);
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