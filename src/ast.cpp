#include "ast.h"

using namespace nanoc;

void ASTNode::recursivePrint(u16 pad) const
{
  // TODO: use single printf
  for (u16 i = 0; i < pad*2; ++i)
    printf(" ");
  print();
  printf("\n");
}

void ASTListRecur::recursivePrint(u16 pad) const
{
  ASTNode::recursivePrint(pad);
  
  const ASTListRecur* next = this;
  
  while (next)
  {
    if (next->item)
      next->item->recursivePrint(pad+1);
    next = next->next.get();
  }
}

void ASTListSeq::recursivePrint(u16 pad) const
{
  ASTNode::recursivePrint(pad);
  for (const auto& child : children)
    child->recursivePrint(pad+1);
}
