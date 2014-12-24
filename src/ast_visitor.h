#ifndef __AST_VISITOR_H__
#define __AST_VISITOR_H__

#include "utils.h"

namespace nanoc
{
  template<typename T>
  class ASTList;
  
  class ASTDeclaration;
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
  class ASTTernaryExpression;
  class ASTReference;
  class ASTArrayReference;
  class ASTCall;
  class ASTWhile;
  class ASTReturn;
  
  class ASTConditionalBlock;
  class ASTIfBlock;
  class ASTElseBlock;
  class ASTConditional;
  
  class ASTAssign;

  class Visitor
  {
  protected:
    virtual void visit(ASTList<ASTStatement>* node);
    virtual void visit(ASTList<ASTExpression>* node);
    virtual void visit(ASTList<ASTDeclaration>* node);
    virtual void visit(ASTList<ASTConditionalBlock>* node);
    virtual void visit(ASTNumber* node);
    virtual void visit(ASTBool* node);
    virtual void visit(ASTReference* node);
    virtual void visit(ASTArrayReference* node);
    virtual void visit(ASTCall* node);
    virtual void visit(ASTTernaryExpression* node);
    virtual void visit(ASTBinaryExpression* node);
    virtual void visit(ASTUnaryExpression* node);
    virtual void visit(ASTAssign* node);
    virtual void visit(ASTReturn* node);
    
    virtual void visit(ASTDeclarationValue<Type::BOOL>* node);
    virtual void visit(ASTDeclarationValue<Type::WORD>* node);
    virtual void visit(ASTDeclarationValue<Type::BYTE>* node);
    
    virtual void visit(ASTConditional* node);
    virtual void visit(ASTIfBlock* node);
    virtual void visit(ASTElseBlock* node);
    
    virtual void visit(ASTFuncDeclaration* node);
    
    virtual void visit(ASTWhile* node);
    
    virtual void leafVisit(ASTNode* node);
    
    virtual void commonVisit(ASTNode* node) { }
    virtual void enteringNode(ASTNode* node) { };
    virtual void exitingNode(ASTNode* node) { };
  
  public:
    virtual void visit(ASTNode* node);

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
    void commonVisit(ASTNode* node);
  };
  
}

#endif