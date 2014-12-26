#ifndef __AST_VISITOR_H__
#define __AST_VISITOR_H__

#include "utils.h"

#define VISITOR_FUNCTIONALITY(__CLASS_NAME__) virtual void visit(__CLASS_NAME__* node);\
virtual void enteringNode(__CLASS_NAME__* node);\
virtual void exitingNode(__CLASS_NAME__* node);

namespace nanoc
{
  template<typename T>
  class ASTList;
  
  class ASTDeclaration;
  template<Type T>
  class ASTDeclarationValue;
  template<Type T>
  class ASTDeclarationArray;
  class ASTFuncDeclaration;
  
  class ASTNode;
  
  class ASTScope;
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
    VISITOR_FUNCTIONALITY(ASTList<ASTStatement>)
    VISITOR_FUNCTIONALITY(ASTList<ASTExpression>)
    VISITOR_FUNCTIONALITY(ASTList<ASTDeclaration>)
    VISITOR_FUNCTIONALITY(ASTList<ASTConditionalBlock>)
    VISITOR_FUNCTIONALITY(ASTFuncDeclaration)
    VISITOR_FUNCTIONALITY(ASTScope)
    VISITOR_FUNCTIONALITY(ASTNumber)
    VISITOR_FUNCTIONALITY(ASTBool)
    VISITOR_FUNCTIONALITY(ASTReference)
    VISITOR_FUNCTIONALITY(ASTArrayReference)
    VISITOR_FUNCTIONALITY(ASTCall)
    VISITOR_FUNCTIONALITY(ASTTernaryExpression)
    VISITOR_FUNCTIONALITY(ASTBinaryExpression)
    VISITOR_FUNCTIONALITY(ASTUnaryExpression)
    VISITOR_FUNCTIONALITY(ASTAssign)
    VISITOR_FUNCTIONALITY(ASTReturn)
    
    VISITOR_FUNCTIONALITY(ASTDeclarationValue<Type::BOOL>)
    VISITOR_FUNCTIONALITY(ASTDeclarationValue<Type::WORD>)
    VISITOR_FUNCTIONALITY(ASTDeclarationValue<Type::BYTE>)
    VISITOR_FUNCTIONALITY(ASTDeclarationArray<Type::BOOL>)
    VISITOR_FUNCTIONALITY(ASTDeclarationArray<Type::WORD>)
    VISITOR_FUNCTIONALITY(ASTDeclarationArray<Type::BYTE>)
    
    VISITOR_FUNCTIONALITY(ASTConditional)
    VISITOR_FUNCTIONALITY(ASTIfBlock)
    VISITOR_FUNCTIONALITY(ASTElseBlock)
    
    
    VISITOR_FUNCTIONALITY(ASTWhile)

    virtual void leafVisit(ASTNode* node);
    
    virtual void commonVisit(ASTNode* node) { }
    virtual void commonEnteringNode(ASTNode* node) { };
    virtual void commonExitingNode(ASTNode* node) { };
  
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
    void commonEnteringNode(ASTNode* node);
    void commonExitingNode(ASTNode* node);
    void commonVisit(ASTNode* node);
  };  
}

#endif