#include "rtl.h"

#include "ast.h"

using namespace std;
using namespace rtl;
using namespace nanoc;

u32 Temporary::counter = 0;

std::string Procedure::mnemonic()
{
  string result = fmt::format("Procedure({}, [", name);
  for (size_t i = 0; i < arguments.size(); ++i)
  {
    const auto& arg = arguments[i];
    result += fmt::format("({}: {})", arg.temporary.getName().c_str(), arg.name.c_str());
    
    if (i < arguments.size()-1)
      result += ", ";
  }
  result += fmt::format("], {})", hasReturnValue ? "yes" : "no");
  
  return result;
}

ASTNode* RTLBuilder::exitingNode(ASTReference* node)
{
  AssignmentInstruction* i = new AssignmentInstruction(value(node->getName()));
  temporaries.push(reference_wrapper<const Temporary>(i->getDestination()));
  currentProcedure->instructions.push_back(unique_ptr<Instruction>(i));
  return nullptr;
}

ASTNode* RTLBuilder::exitingNode(ASTNumber* node)
{
  AssignmentInstruction* i = new AssignmentInstruction(value((u16)node->getValue()));
  temporaries.push(reference_wrapper<const Temporary>(i->getDestination()));
  currentProcedure->instructions.push_back(unique_ptr<Instruction>(i));
  return nullptr;
}

ASTNode* RTLBuilder::exitingNode(ASTBinaryExpression *node)
{
  auto src2 = temporaries.top();
  temporaries.pop();
  auto src1 = temporaries.top();
  temporaries.pop();
  
  rtl::Operation op = rtl::Operation::ADDITION;
  //switch (node->getOperand2() { ... }
  
  OperationInstruction* i = new OperationInstruction(src1, src2, op);
  temporaries.push(reference_wrapper<const Temporary>(i->getDestination()));
  currentProcedure->instructions.push_back(unique_ptr<Instruction>(i));
  return nullptr;
}

ASTNode* RTLBuilder::exitingNode(ASTCall* node)
{
  size_t count = node->getArguments()->getElements().size();
  vector<reference_wrapper<const Temporary>> arguments;
  
  for (int i = 0; i < count; ++i)
  {
    auto ref = temporaries.top();
    arguments.insert(arguments.begin(), ref);
  }

  CallInstruction* i;
  
  /*if (node->getType()->isVoid())
    i = new CallInstruction(node->getName(), arguments);
  else*/
    i = new CallInstruction(node->getName(), arguments, true);

  currentProcedure->instructions.push_back(unique_ptr<Instruction>(i));
  return nullptr;
}

void RTLBuilder::enteringNode(nanoc::ASTFuncDeclaration* node)
{
  Procedure *procedure = new Procedure();
  
  procedure->name = node->getName();
  procedure->hasReturnValue = !node->getReturnType()->isVoid();
  
  for (const auto& arg : node->getArguments())
  {
    procedure->arguments.push_back({arg.name, Temporary()});
  }
  
  code.push_back(unique_ptr<Procedure>(procedure));
  this->currentProcedure = procedure;
}

void RTLBuilder::print()
{
  for (const auto& proc : code)
  {
    printf("%s\n", proc->mnemonic().c_str());
    
    for (const auto& i : proc->instructions)
    {
      printf("  %s\n", i->mnemonic().c_str());
    }
  }
}