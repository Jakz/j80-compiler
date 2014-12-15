#include "assembler.h"

using namespace std;

u16 Assembler::position = 0;

list<Instruction> Assembler::instructions;

vector<pair<u16, std::string> > Assembler::jumps;
vector<std::pair<u16, DataReference> > Assembler::dataReferences;

unordered_map<string,u16> Assembler::labels;
unordered_map<std::string, DataSegmentEntry> Assembler::data;

DataSegment* Assembler::dataSegment = new DataSegment();
CodeSegment* Assembler::codeSegment = new CodeSegment();

s8 Assembler::currentIrq = -1;
std::list<Instruction> Assembler::irqs[4];


constexpr BinaryCode Assembler::INVALID;

void Assembler::init()
{
  instructions.clear();
  position = 0;
}

void Assembler::assemble(int opcode, int opcode2, int opcode3)
{

}

void Assembler::placeLabel(char *label)
{
  labels[label] = position;
}

bool Assembler::solveJumps()
{
  for (auto &jump : jumps)
  {
    unordered_map<std::string, u16>::iterator it = labels.find(jump.second);
    
    if (it != labels.end())
    {
      u16 realAddress = codeSegment->offset + it->second;

      printf("Jump to %s at address %04X - %p\n", jump.second.c_str(), it->second, &jump.first);
      codeSegment->data[jump.first+1] = realAddress & 0xFF;
      codeSegment->data[jump.first+2] = (realAddress >> 8) & 0xFF;
    }
    else
    {
      printf("Label %s unresolved!\n", jump.second.c_str());
      return false;
    }
  }
  
  return true;
}

/*u16 Assembler::size()
{
  return position;
}*/

BinaryCode Assembler::consolidate()
{
  BinaryCode pack;
  pack.code = nullptr;
  pack.length = position;

  return pack;
}