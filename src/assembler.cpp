#include "assembler.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace Assembler;

constexpr BinaryCode J80Assembler::INVALID;

J80Assembler::J80Assembler() : currentIrq(-1), dataSegment(new DataSegment()), codeSegment(new CodeSegment()), position(0)
{
  
}

bool J80Assembler::parse(const std::string &filename)
{
  instructions.clear();
  position = 0;
  
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

void J80Assembler::assemble(int opcode, int opcode2, int opcode3)
{

}

void J80Assembler::placeLabel(const std::string& label)
{
  labels[label] = position;
}

bool J80Assembler::solveJumps()
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

void J80Assembler::error (const Assembler::location& l, const std::string& m)
{
  cerr << "Assembler error at " << file << ":" << l.begin.line << "," << l.begin.column << " : " << m << endl;
}

void J80Assembler::error (const std::string& m)
{
  cerr << "Assembler error: " << m << endl;
}

/*u16 Assembler::size()
{
  return position;
}*/

BinaryCode J80Assembler::consolidate()
{
  BinaryCode pack;
  pack.code = nullptr;
  pack.length = position;

  return pack;
}