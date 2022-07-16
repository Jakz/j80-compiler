#include "rtl.h"

#include "ast.h"

#include <sstream>

using namespace std;
using namespace rtl;
using namespace nanoc;

u32 Temporary::counter = 0;

template<>
struct fmt::formatter<Temporary> : fmt::formatter<std::string>
{
  template<typename Context>
  auto format(const Temporary& c, Context& ctx)
  {
    return format_to(ctx.out(), "t{}", c.i());
  }
};

template<>
struct fmt::formatter<std::reference_wrapper<const Temporary>> : fmt::formatter<std::string>
{
  template<typename Context>
  auto format(const std::reference_wrapper<const Temporary>& c, Context& ctx)
  {
    return format_to(ctx.out(), "t{}", c.get().i());
  }
};

template<>
struct fmt::formatter<rtl::Argument> : fmt::formatter<std::string>
{
  template<typename Context>
  auto format(const rtl::Argument& c, Context& ctx)
  {
    return format_to(ctx.out(), "({} : {})", c.temporary, c.name);
  }
};


template<typename... Args> void log(const std::string& format, Args&&... args)
{
  printf("%s\n", fmt::format(format, std::forward<Args>(args)...).c_str());
}


InstructionBlock::InstructionBlock()
{
  static size_t blockCounter = 0;
  index = blockCounter++;
}

void Procedure::buildCFG()
{
  std::unique_ptr<InstructionBlock> block = std::move(blocks.front());
  blocks.pop_back();
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
    for (size_t i = 0; i < blocks.size(); ++i)
    {
      const auto& block = blocks[i];
      auto* last = block->last();
      
      const Return* retn = dynamic_cast<const Return*>(last);
      const Jump* jump = dynamic_cast<const Jump*>(last);
      const ConditionalJump* cjump = dynamic_cast<const ConditionalJump*>(last);



      /* add link to next block which always happens unless RETN or JUMP instruction */
      if ((cjump || (!jump && !retn)) && i < blocks.size() - 1)
        block->link(blocks[i + 1].get());

      /* add link to label destination if last instruction is a jump */
      if (jump)
      {
        auto it = std::find_if(blocks.begin(), blocks.end(), [jl = jump->label()](const auto& block) {
          const Label* label = dynamic_cast<const Label*>(block->first());
          return label && label->label() == jl;
        });

        if (it != blocks.end())
          block->link(it->get());
      }
    }
  }
}

void Procedure::liveAnalysis()
{
  bool finished = false;

  log("starting live-in analysis");

  log("  computing def and use sets");

  for (const auto& block : blocks)
  {
    for (const auto& rtl : *block)
    {
      const Assignment* assign = dynamic_cast<const Assignment*>(rtl.get());
      const OperationInstruction* operation = dynamic_cast<const OperationInstruction*>(rtl.get());

      /* add dest to def and add src to use if it's temporary unless it was in def */
      if (assign)
      {
        block->live.def.push_back(std::reference_wrapper<const Temporary>(assign->destination()));

        if (assign->value().type == value::Type::Temp && !block->live.isDefined(assign->value().temp))
          block->live.use.push_back(std::reference_wrapper<const Temporary>(assign->value().temp));
      }
      else if (operation)
      {
        block->live.def.push_back(std::reference_wrapper<const Temporary>(operation->destination()));

        if (!block->live.isDefined(operation->operand1()))
          block->live.use.push_back(std::reference_wrapper<const Temporary>(operation->operand1()));

        if (!block->live.isDefined(operation->operand2()))
          block->live.use.push_back(std::reference_wrapper<const Temporary>(operation->operand2()));
      }

    }
  }

  for (const auto& block : blocks)
  {
    std::stringstream ss;

    log("    block {}", block->index);
    log("      def {}", fmt::join(block->live.def, " "));
    log("      use {}", fmt::join(block->live.use, " "));
  }

  /* compute use and def */
  while (!finished)
  {
    finished = true;

   
  }

}

std::string Procedure::mnemonic()
{
  return fmt::format("Procedure({}, [{}], {}", 
    name, 
    fmt::join(arguments, ", "), 
    hasReturnValue ? "yes" : "no"
  );
}

ASTNode* RTLBuilder::exitingNode(ASTReference* node)
{
  Assignment* i = new Assignment(value(node->getName()));
  temporaries.push(reference_wrapper<const Temporary>(i->destination()));
  add(i);
  return nullptr;
}

ASTNode* RTLBuilder::exitingNode(ASTNumber* node)
{
  Assignment* i = new Assignment(value((u16)node->getValue()));
  temporaries.push(reference_wrapper<const Temporary>(i->destination()));
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
  temporaries.push(reference_wrapper<const Temporary>(i->destination()));
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
  currentProcedure->liveAnalysis();
  return nullptr;
}

void RTLBuilder::computeConstants()
{
  for (const auto& procedure : code)
  {
    for (const auto& block : procedure->blocks)
    {
      for (const auto& i : *block)
      {
        Assignment* assignment = dynamic_cast<Assignment*>(i.get());

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
    
    for (const auto& b : proc->blocks)
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