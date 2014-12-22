#include "assembler.h"

#include <iostream>
#include <fstream>

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
  
  Assembler::Lexer lexer = Assembler::Lexer(*this, &is);
  Assembler::Parser parser(lexer, *this);
  parser.set_debug_level(shouldGenerateTrace);
  int res = parser.parse();
  return res == 0;
}

void J80Assembler::error (const Assembler::location& l, const std::string& m)
{
  cerr << "Assembler error at " << file << ":" << l.begin.line << "," << l.begin.column << " : " << m << endl;
}

void J80Assembler::error (const std::string& m)
{
  cerr << "Assembler error: " << m << endl;
}

void J80Assembler::printProgram() const
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
    
    printf("%04X: ", address);
    
    const u16 length = i->getLength();
    i->assemble(opcode);
    
    for (int i = 0; i < length; ++i)
      printf("%02X", opcode[i]);
    for (int i = length; i < LENGTH_4_BYTES+1; ++i)
      printf("  ");
    
    const std::string mnemonic = i->mnemonic();
    
    if (mnemonic.empty())
      Opcodes::printInstruction(opcode);
    
    printf("%s", mnemonic.c_str());
    
    if (keepLabels)
    {
      for (int x = 0; x < 40 - mnemonic.length(); ++x)
        printf(" ");
      
      if (label)
        printf("<%s>", label->getLabel().c_str());
      
    }
    
    printf("\n");

    address += length;
    
    label = nullptr;
  }
  
  u16 dataLen = dataSegment.length;
  for (int i = 0; i < dataLen / 8 + (dataLen % 8 != 0 ? 1 : 0); ++i)
  {
    printf("%04X: ", address + i*8);
    
    for (int j = 0; j < 8; ++j)
    {
      if (i*8+j < dataLen)
        printf("%02X", dataSegment.data[i*8 + j]);
      else
        printf("  ");
    }
    
    printf(" ");
    
    for (int j = 0; j < 8; ++j)
    {
      if (i*8+j < dataLen &&  dataSegment.data[i*8 + j] >= 0x20 && dataSegment.data[i*8 + j] <= 0x7E)
        printf("%c", dataSegment.data[i*8 + j]);
      else
        printf(" ");
    }
    
    printf("\n");
  }
}

void J80Assembler::buildDataSegment()
{
  printf("Building data segment.\n");
  u16 totalSize = 0;
  for (auto &entry : data)
    totalSize += entry.second.length;
  
  printf("  Data segment size: %u bytes.\n", totalSize);
  
  dataSegment.alloc(totalSize);
  
  totalSize = 0;
  
  for (auto &entry : data)
  {
    memcpy(&dataSegment.data[totalSize], entry.second.data, entry.second.length);
    entry.second.offset = totalSize;
    
    printf("  > Data %s at offset %.4Xh\n",entry.first.c_str(),entry.second.offset);
    
    totalSize += entry.second.length;
  }
}

void J80Assembler::buildCodeSegment()
{
  printf("Building code segment.\n");
  
  u16 totalSize = 0;
  
  std::list<std::unique_ptr<Instruction>>::const_iterator it = iterator();

  while (hasNext(it))
  {
    totalSize += (*it)->getLength();
    ++it;
  }
  
  printf("  Code segment total size: %u bytes (%lu entries).\n", totalSize, instructions.size());
  
  if (entryPoint.isSet())
    printf("  Entry point specified at %.4Xh.\n", entryPoint.get());
  
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
    instructions.insert(++iteratorToMainLabel, unique_ptr<Instruction>(new InstructionLD_NNNN(REG_SP, stackBase.get())));
  
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
    
    instructions.push_front(unique_ptr<Instruction>(new InstructionJMP_NNNN(COND_UNCOND, "main")));
  }
}


bool J80Assembler::solveJumps()
{
  
  printf("Computing label addresses.\n");
  
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
      printf("  > Label %s resolved to address %04Xh\n", label->getLabel().c_str(), address);
    }
    else if (intEntryPoint && intEntryPoint->mustBeSolved())
    {
      intEntryPoint->solve(address);
      interrupts[intEntryPoint->getIndex()].set(address);
      printf("  > Interrupt %d resolved to address %04Xh\n", intEntryPoint->getIndex(), address);
    }
    else
      address += i->getLength();
  }

  printf("Solving jumps.\n");
  
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
          printf("  Label %s unresolved.\n", ai->getLabel().c_str());
          return false;
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
          printf ("  Interrupt entry for %d unresolved.\n", ai->getIntIndex());
        }
      }
    }
  }
  
  return true;
}

void J80Assembler::solveDataReferences()
{  
  u16 base = computeDataSegmentOffset();

  printf("Solving data references. Base data segment offset: %d.\n", base);
  
  for (const auto& i : instructions)
  {
    InstructionLD_NN* vi = dynamic_cast<InstructionLD_NN*>(i.get());
    InstructionLD_NNNN* vi16 = dynamic_cast<InstructionLD_NNNN*>(i.get());
    
    if (vi && vi->mustBeSolved())
    {
      std::unordered_map<std::string, DataSegmentEntry>::const_iterator it = data.find(vi->getLabel());
      
      if (it != data.end())
      {
        vi->solve(it->second.length);
        printf("  > Data '%s' length\n", vi->getLabel().c_str());

      }
    }
    else if (vi16 && vi16->mustBeSolved())
    {
      std::unordered_map<std::string, DataSegmentEntry>::const_iterator it = data.find(vi16->getLabel());

      if (it != data.end())
      {
        vi16->solve(base + it->second.offset);
        printf("  > Data '%s' at %.4Xh\n", vi16->getLabel().c_str(), base + it->second.offset);
      }
    }
  }
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