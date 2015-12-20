#ifndef __AST_VISITOR_H__
#define __AST_VISITOR_H__

#include "utils.h"

#define VISITOR_FUNCTIONALITY(__CLASS_NAME__) virtual ASTNode* visit(__CLASS_NAME__* node);\
virtual void enteringNode(__CLASS_NAME__* node);\
virtual ASTNode* exitingNode(__CLASS_NAME__* node);

namespace nanoc
{
  template<typename T>
  class ASTList;
  
  class ASTDeclaration;
  class ASTDeclarationValue;
  class ASTDeclarationArray;
  class ASTFuncDeclaration;
  class ASTEnumDeclaration;
  class ASTStructDeclaration;
  
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
  class ASTDereference;
  class ASTAddressOf;
  class ASTFieldAccess;
  class ASTCall;
  class ASTWhile;
  class ASTReturn;
  class ASTLeftHand;
  
  class ASTConditionalBlock;
  class ASTIfBlock;
  class ASTElseBlock;
  class ASTConditional;
  
  class ASTAssign;
  
  class ASTEnumEntry;
  class ASTStructField;

  class Visitor
  {
  protected:
    VISITOR_FUNCTIONALITY(ASTList<ASTStatement>)
    VISITOR_FUNCTIONALITY(ASTList<ASTExpression>)
    VISITOR_FUNCTIONALITY(ASTList<ASTDeclaration>)
    VISITOR_FUNCTIONALITY(ASTList<ASTConditionalBlock>)
    VISITOR_FUNCTIONALITY(ASTList<ASTEnumEntry>)
    VISITOR_FUNCTIONALITY(ASTList<ASTStructField>)
    VISITOR_FUNCTIONALITY(ASTFuncDeclaration)
    VISITOR_FUNCTIONALITY(ASTEnumDeclaration)
    VISITOR_FUNCTIONALITY(ASTStructDeclaration)
    VISITOR_FUNCTIONALITY(ASTScope)
    VISITOR_FUNCTIONALITY(ASTNumber)
    VISITOR_FUNCTIONALITY(ASTBool)
    VISITOR_FUNCTIONALITY(ASTReference)
    VISITOR_FUNCTIONALITY(ASTArrayReference)
    VISITOR_FUNCTIONALITY(ASTCall)
    VISITOR_FUNCTIONALITY(ASTTernaryExpression)
    VISITOR_FUNCTIONALITY(ASTBinaryExpression)
    VISITOR_FUNCTIONALITY(ASTUnaryExpression)
    VISITOR_FUNCTIONALITY(ASTFieldAccess)
    VISITOR_FUNCTIONALITY(ASTDereference)
    VISITOR_FUNCTIONALITY(ASTAddressOf)
    VISITOR_FUNCTIONALITY(ASTAssign)
    VISITOR_FUNCTIONALITY(ASTReturn)
    VISITOR_FUNCTIONALITY(ASTLeftHand)
    
    VISITOR_FUNCTIONALITY(ASTDeclarationValue)
    VISITOR_FUNCTIONALITY(ASTDeclarationArray)
    
    VISITOR_FUNCTIONALITY(ASTConditional)
    VISITOR_FUNCTIONALITY(ASTIfBlock)
    VISITOR_FUNCTIONALITY(ASTElseBlock)
    
    VISITOR_FUNCTIONALITY(ASTWhile)
    
    VISITOR_FUNCTIONALITY(ASTEnumEntry)
    VISITOR_FUNCTIONALITY(ASTStructField)
    
    template<typename T>
    void dispatchAndReplace(std::unique_ptr<T>& ptr);
    
    virtual void commonVisit(ASTNode* node) { }
    virtual void commonEnteringNode(ASTNode* node) { };
    virtual void commonExitingNode(ASTNode* node) { };
  
  public:
    virtual ASTNode* dispatch(ASTNode* node);

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