#include "assembler.h"

#include <iostream>
#include <fstream>
#include <cstdarg>

#include "support/format/format.h"

using namespace std;
using namespace Assembler;

J80Assembler::J80Assembler() : dataSegment(DataSegment()), codeSegment(CodeSegment()), position(0)
{
  
}

bool J80Assembler::parse(const std::string &filename)
{
  entryPoint = Optional<u16>();
  
  instructions.clear();
  dataReferences.clear();
  data.clear();
  
  position = 0;
  dataSegment = DataSegment();
  codeSegment = CodeSegment();

  file = filename;
  
  bool shouldGenerateTrace = false;
  
  ifstream is;
  is.open(filename);
  
  Assembler::Lexer lexer(*this, &is);
  Assembler::Parser parser(lexer, *this);
  parser.set_debug_level(shouldGenerateTrace);
  int res = parser.parse();
  return res == 0;
}

void J80Assembler::error (const Assembler::location& l, const std::string& m)
{
  log(Log::ERROR, true, "Assembler error at {}: {},{} : {}", file, l.begin.line, l.begin.column, m);
}

void J80Assembler::error (const std::string& m)
{
  log(Log::ERROR, true, "Assembler error: {}", m);
}

void J80Assembler::printProgram(std::ostream& out) const
{
  bool keepLabels = true;

  u16 address = 0;
  u8 opcode[4];

  const Label* label = nullptr;
  
  for (const auto& i : instructions)
  {
    if (!i->isReal())
    {
      label = dynamic_cast<Label*>(i.get());
      continue;
    }
    
    out << fmt::format("{:04X}: ", address);
    
    const u16 length = i->getLength();
    i->assemble(opcode);
    
    for (int i = 0; i < length; ++i)
      out << fmt::format("{:02X}", opcode[i]);
    for (int i = length; i < 4+1; ++i)
      out << fmt::format("  ");
    
    std::string mnemonic = i->mnemonic();
    
    out << fmt::format("{}", mnemonic);
    
    if (keepLabels)
    {
      for (int x = 0; x < 40 - mnemonic.length(); ++x)
        out << fmt::format(" ");
      
      if (label)
        out << fmt::format("<{}>", label->getLabel());
      
    }
    
    out << fmt::format("\n");

    address += length;
    
    label = nullptr;
  }
  
  u16 dataLen = dataSegment.length;
  for (int i = 0; i < dataLen / 8 + (dataLen % 8 != 0 ? 1 : 0); ++i)
  {
    out << fmt::format("{:04X}: ", address + i*8);
    
    for (int j = 0; j < 8; ++j)
    {
      if (i*8+j < dataLen)
        out << fmt::format("{:02X}", dataSegment.data[i*8 + j]);
      else
        out << fmt::format("  ");
    }
    
    out << fmt::format(" ");
    
    for (int j = 0; j < 8; ++j)
    {
      if (i*8+j < dataLen &&  dataSegment.data[i*8 + j] >= 0x20 && dataSegment.data[i*8 + j] <= 0x7E)
        out << fmt::format("{}", (char)dataSegment.data[i*8 + j]);
      else
        out << fmt::format(" ");
    }
    
    out << fmt::format("\n");
  }
}

void J80Assembler::buildDataSegment()
{
  /* for each entry specified as data compute total size in bytes of segment */
  u16 totalSize = 0;
  for (auto &entry : data)
    totalSize += entry->second.length;
  
  log(Log::INFO, true, "Building data segment, total size: {} bytes", totalSize);
  
  dataSegment.alloc(totalSize);
  
  totalSize = 0;
  
  /* copy data from data entry to final data segment at correct offset and save
     the offset to solve references to it when assembling code
   */
  for (auto &entry : data)
  {
    const auto* data = entry->second.getData();
    std::copy(data, data + entry->second.length, &dataSegment.data[totalSize]);
    entry->second.offset = totalSize;
    
    log(Log::VERBOSE_INFO, true, "  > Data {} ({} bytes) at offset {:4X}h", entry->first, entry->second.length, entry->second.offset);
    
    totalSize += entry->second.length;
  }
}

void J80Assembler::buildCodeSegment()
{
  u16 totalSize = 0, instructionCount = 0;
  
  std::list<std::unique_ptr<Instruction>>::const_iterator it = iterator();

  while (hasNext(it))
  {
    u16 length = (*it)->getLength();
    totalSize += length;
    instructionCount += length != 0 ? 1 : 0;
    ++it;
  }
  
  log(Log::INFO, true, "Building code segment, total size: {} bytes in {} instruction", totalSize, instructionCount);
  
  if (entryPoint.isSet())
    log(Log::VERBOSE_INFO, true, "  Entry point specified at {:4X}h.", entryPoint.get());
  
  codeSegment.alloc(totalSize+codeSegment.offset);
  totalSize = 0;
  
  it = iterator();
  while (hasNext(it))
  {
    (*it)->assemble(&codeSegment.data[totalSize+codeSegment.offset]);
    totalSize += (*it)->getLength();
    ++it;
  }
}

void J80Assembler::prepareSource()
{
  bool hasAtLeastOneInterrupt = false;
  for (int i = 0; i < maxNumberOfInterrupts(); ++i)
    hasAtLeastOneInterrupt |= irqs[i];
  
  /* search for a label called "main" as the entry point of the program */
  bool hasMainLabel = false;
  auto iteratorToMainLabel = instructions.begin();
  for ( ; iteratorToMainLabel != instructions.end(); ++iteratorToMainLabel)
  {
    const Label* label = dynamic_cast<Label*>((*iteratorToMainLabel).get());
    
    if (label && label->getLabel() == "main")
    {
      hasMainLabel = true;
      break;
    }
  }
  
  /* if program doesn't have explicit entry point then add one at the beginning */
  if (!hasMainLabel)
  {
    instructions.push_front(unique_ptr<Instruction>(new Label("main")));
    iteratorToMainLabel = instructions.begin();
  }
  
  /* if program specifies a stack base then add a LD SP, NNNN instruction */
  /* TODO: this is not language agnostic */
  if (stackBase.isSet())
    instructions.insert(++iteratorToMainLabel, unique_ptr<Instruction>(new InstructionLD_NNNN(Reg::SP, stackBase.get())));
  
  /* if program has at least one interrupt we need to place a jump at the beginning to entry point
     and setup interrupt vector table */
  if (hasAtLeastOneInterrupt)
  {
    for (int i = maxNumberOfInterrupts() - 1; i >= 0; --i)
    {
      if (irqs[i])
      {
        instructions.push_front(unique_ptr<Instruction>(new InstructionNOP()));
        instructions.push_front(unique_ptr<Instruction>(new InstructionJMP_NNNN(COND_UNCOND, (InterruptIndex)i)));
      }
      else
        instructions.push_front(unique_ptr<Instruction>(new Padding(4)));
    }
    
    const u16 INTERRUPT_VECTOR_BASE = 0b10000;
    
    for (int i = 0; i < INTERRUPT_VECTOR_BASE - 3; ++i)
      instructions.push_front(unique_ptr<Instruction>(new InstructionNOP()));
    
    instructions.push_front(unique_ptr<Instruction>(new InstructionJMP_NNNN(COND_UNCOND, Address("main"))));
  }
}


Result J80Assembler::solveJumps()
{
  log(Log::INFO, true, "Computing label addresses.");
  
  std::vector<Optional<u16>> interrupts(maxNumberOfInterrupts());
  
  unordered_map<std::string, u16> labels;
  
  u16 address = 0;
  for (const auto &i : instructions)
  {
    Label* label = dynamic_cast<Label*>(i.get());
    InterruptEntryPoint* intEntryPoint = dynamic_cast<InterruptEntryPoint*>(i.get());

    if (label && label->mustBeSolved())
    {
      label->solve(address);
      labels[label->getLabel()] = address;
      log(Log::VERBOSE_INFO, true, "  > Label {} resolved to address {:04X}h", label->getLabel(), address);
    }
    else if (intEntryPoint && intEntryPoint->mustBeSolved())
    {
      intEntryPoint->solve(address);
      interrupts[intEntryPoint->getIndex()].set(address);
      log(Log::VERBOSE_INFO, true, "  > Interrupt {} resolved to address {:04X}h", intEntryPoint->getIndex(), address);
    }
    else
      address += i->getLength();
  }

  log(Log::INFO, true, "Solving jumps.");
  
  for (const auto &i : instructions)
  {
    InstructionAddressable* ai = dynamic_cast<InstructionAddressable*>(i.get());

    /* if instruction has an address that could be a label and it must be solved */
    if (ai && ai->mustBeSolved())
    {
      if (ai->getType() == Address::Type::LABEL)
      {
        // find address for label
        unordered_map<std::string, u16>::iterator it = labels.find(ai->getLabel());

        if (it != labels.end())
        {
          //printf("  > Jump to %s at address %04Xh\n", ai->getLabel().c_str(), it->second);
          u16 realAddress = codeSegment.offset + it->second;
          ai->solve(realAddress);
        }
        else
        {
          //log(Log::ERROR, true, "  Label %s unresolved.", );
          return Result(fmt::format("Label {} unresolved.", ai->getLabel()));
        }
      }
      else if (ai->getType() == Address::Type::INTERRUPT)
      {
        const Optional<u16>& address = interrupts[ai->getIntIndex()];
        if (address.isSet())
        {
          u16 realAddress = codeSegment.offset + address.get();
          ai->solve(realAddress);
        }
        else
        {
          log(Log::ERROR, true, "  Interrupt entry for {} unresolved.", ai->getIntIndex());
        }
      }
    }
  }
  
  return Result();
}

Result J80Assembler::solveDataReferences()
{  
  u16 base = computeDataSegmentOffset();

  log(Log::INFO, true, "Solving data references. Base data segment offset: {:04X}h.", base);
  
  Environment env{ *this, data, consts, base };
  
  for (const auto& i : instructions)
  {
    Result result = i->solve(env);
    
    if (!result)
      return result;
  }
  
  return Result();
}

void J80Assembler::saveForLogisim(const std::string &filename) const
{
  FILE *bin = fopen(filename.c_str(), "wb");
  fprintf(bin, "v2.0 raw\n");
  
  for (int i = 0; i < codeSegment.length; ++i)
    fprintf(bin, "%02x\n", codeSegment.data[i]);
  
  for (int i = 0; i < dataSegment.length; ++i)
    fprintf(bin, "%02x\n", dataSegment.data[i]);
  
  fclose(bin);
}

void J80Assembler::saveBinary(const std::string &filename) const
{
  FILE *bin = fopen(filename.c_str(), "wb");
  
  fwrite(codeSegment.data, sizeof(u8), codeSegment.length, bin);
  fwrite(dataSegment.data, sizeof(u8), dataSegment.length, bin);
  
  fclose(bin);
}
