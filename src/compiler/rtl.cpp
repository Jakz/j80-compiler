#include "rtl.h"

#include "ast.h"

#include <sstream>

using namespace std;
using namespace rtl;
using namespace nanoc;

u32 Temporary::counter = 0;



InstructionBlock::InstructionBlock()
{
  static size_t blockCounter = 0;
  index = blockCounter++;
}

void Procedure::buildCFG()
{
  std::unique_ptr<InstructionBlock> block = std::move(instructions.front());
  instructions.pop_back();
  InstructionBlock* current = addBlock();

  for (auto& i : *block)
  {
    Branch branch = i->branch();
    
    if (branch == Branch::Before)
    {
      current = addBlock();
    }
    
    current->add(i.release());

    if (branch == Branch::After)
    {
      current = addBlock();

    }
  }

  /* map jumps and labels to link control flow graph */
  {
    for (size_t i = 0; i < instructions.size(); ++i)
    {
      const auto& block = instructions[i];
      auto* last = block->last();
      
      const Return* retn = dynamic_cast<const Return*>(last);
      const Jump* jump = dynamic_cast<const Jump*>(last);
      const ConditionalJump* cjump = dynamic_cast<const ConditionalJump*>(last);



      /* add link to next block which always happens unless RETN or JUMP instruction */
      if ((cjump || (!jump && !retn)) && i < instructions.size() - 1)
        block->link(instructions[i + 1].get());

      /* add link to label destination if last instruction is a jump */
      if (jump)
      {
        auto it = std::find_if(instructions.begin(), instructions.end(), [jl = jump->label()](const auto& block) {
          const Label* label = dynamic_cast<const Label*>(block->first());
          return label && label->label() == jl;
        });

        if (it != instructions.end())
          block->link(it->get());
      }
    }
  }
}

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
  add(i);
  return nullptr;
}

ASTNode* RTLBuilder::exitingNode(ASTNumber* node)
{
  AssignmentInstruction* i = new AssignmentInstruction(value((u16)node->getValue()));
  temporaries.push(reference_wrapper<const Temporary>(i->getDestination()));
  add(i);
  return nullptr;
}

ASTNode* RTLBuilder::exitingNode(ASTBinaryExpression* node)
{
  auto src2 = temporaries.top();
  temporaries.pop();
  auto src1 = temporaries.top();
  temporaries.pop();
  
  OperationInstruction* i = new OperationInstruction(src1, src2, node->getOperation());
  temporaries.push(reference_wrapper<const Temporary>(i->getDestination()));
  add(i);
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

  add(i);
  return nullptr;
}

nanoc::ASTNode* RTLBuilder::exitingNode(nanoc::ASTReturn* node)
{
  add(new Return(temporaries.top()));
  temporaries.pop();
  return nullptr;
}

void RTLBuilder::stepNode(ASTIfBlock* node)
{
  auto cond = temporaries.top();
  add(new ConditionalJump(label("jump", ifLabelCounter), cond.get(), true));
}

ASTNode* RTLBuilder::exitingNode(ASTIfBlock* node)
{
  add(new Label(label("jump", ifLabelCounter++)));
  return nullptr;
}

void RTLBuilder::enteringNode(ASTFuncDeclaration* node)
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

nanoc::ASTNode* RTLBuilder::exitingNode(nanoc::ASTFuncDeclaration* node)
{
  currentProcedure->buildCFG();
  return nullptr;
}

void RTLBuilder::computeConstants()
{
  for (const auto& procedure : code)
  {
    for (const auto& block : procedure->instructions)
    {
      for (const auto& i : *block)
      {
        AssignmentInstruction* assignment = dynamic_cast<AssignmentInstruction*>(i.get());

        /* if value is constant then also temporary is constant */
        if (assignment)
        {

        }
      }
    }
  }
}

void RTLBuilder::print()
{
  for (const auto& proc : code)
  {
    printf("%s\n", proc->mnemonic().c_str());
    
    for (const auto& b : proc->instructions)
    {
      std::stringstream ss;

      if (!b->outgoing.empty())
      {
        ss << " -> ";
        for (auto* out : b->outgoing)
          ss << out->index << " ";
      }

      printf("Block %d%s\n", b->index, ss.str().c_str());

      for (const auto& i : *b)
      {
        printf("  %s\n", i->mnemonic().c_str());
      }
    }
  }
}