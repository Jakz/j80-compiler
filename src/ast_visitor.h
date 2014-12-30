#ifndef __AST_VISITOR_H__
#define __AST_VISITOR_H__

#include "utils.h"

#define VISITOR_FUNCTIONALITY(__CLASS_NAME__) virtual void visit(std::unique_ptr<__CLASS_NAME__>& node);\
virtual void enteringNode(std::unique_ptr<__CLASS_NAME__>& node);\
virtual void exitingNode(std::unique_ptr<__CLASS_NAME__>& node);

namespace nanoc
{
  template<typename T>
  class ASTList;
  
  class ASTDeclaration;
  class ASTDeclarationValue;
  class ASTDeclarationArray;
  class ASTFuncDeclaration;
  class ASTEnumDeclaration;
  
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
  
  class ASTEnumEntry;

  class Visitor
  {
  protected:
    VISITOR_FUNCTIONALITY(ASTList<ASTStatement>)
    VISITOR_FUNCTIONALITY(ASTList<ASTExpression>)
    VISITOR_FUNCTIONALITY(ASTList<ASTDeclaration>)
    VISITOR_FUNCTIONALITY(ASTList<ASTConditionalBlock>)
    VISITOR_FUNCTIONALITY(ASTList<ASTEnumEntry>)
    VISITOR_FUNCTIONALITY(ASTFuncDeclaration)
    VISITOR_FUNCTIONALITY(ASTEnumDeclaration)
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
    
    VISITOR_FUNCTIONALITY(ASTDeclarationValue)
    VISITOR_FUNCTIONALITY(ASTDeclarationArray)
    
    VISITOR_FUNCTIONALITY(ASTConditional)
    VISITOR_FUNCTIONALITY(ASTIfBlock)
    VISITOR_FUNCTIONALITY(ASTElseBlock)
    
    VISITOR_FUNCTIONALITY(ASTWhile)
    
    VISITOR_FUNCTIONALITY(ASTEnumEntry)
    
    virtual void commonVisit(std::unique_ptr<ASTNode>& node) { }
    virtual void commonEnteringNode(std::unique_ptr<ASTNode>& node) { };
    virtual void commonExitingNode(std::unique_ptr<ASTNode>& node) { };
  
  public:
    virtual void dispatch(std::unique_ptr<ASTNode>& node);

  };
    
  
  class PrinterVisitor : public Visitor
  {
  private:
    u16 indent = 0;
    
    void pad(u16 indent);
    
  public:
    PrinterVisitor() : indent(0) { }
    void commonEnteringNode(std::unique_ptr<ASTNode>& node);
    void commonExitingNode(std::unique_ptr<ASTNode>& node);
    void commonVisit(std::unique_ptr<ASTNode>& node);
  };  
}

#endif