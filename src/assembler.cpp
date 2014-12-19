#include "assembler.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace Assembler;

J80Assembler::J80Assembler() : currentIrq(-1), dataSegment(DataSegment()), codeSegment(CodeSegment()), position(0)
{
  
}

bool J80Assembler::parse(const std::string &filename)
{
  entryPoint = Optional<u16>();
  
  instructions.clear();
  labels.clear();
  jumps.clear();
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

void J80Assembler::placeLabel(const std::string& label)
{
  labels[label] = position;
}

bool J80Assembler::solveJumps()
{
  printf("Computing jump addresses.\n");
  
  for (auto &jump : jumps)
  {
    unordered_map<std::string, u16>::iterator it = labels.find(jump.second);
    
    if (it != labels.end())
    {
      u16 realAddress = codeSegment.offset + it->second;

      printf("  > Jump to %s at address %04Xh\n", jump.second.c_str(), it->second);
      codeSegment.data[jump.first+codeSegment.offset+2] = realAddress & 0xFF;
      codeSegment.data[jump.first+codeSegment.offset+1] = (realAddress >> 8) & 0xFF;
    }
    else
    {
      printf("  Label %s unresolved!\n", jump.second.c_str());
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

void J80Assembler::printProgram() const
{
  u16 pos = 0;
  char buffer[64];
  
  bool keepLabels = true;

  while (pos < codeSegment.length)
  {
    printf("%04X: ", pos);
    
    u8 length = Opcodes::printInstruction(codeSegment.data+pos, buffer);
    
    for (int w = 0; w < length; ++w)
    {
      printf("%02X", codeSegment.data[pos+w]);
    }
    //fprintf(bin,"\n");
    
    if (length == 1) printf("      ");
    if (length == 2) printf("    ");
    if (length == 3) printf("  ");
    printf("  ");
    
    printf("%s", buffer);
    
    if (keepLabels)
    {
      for (int x = 0; x < 20 - strlen(buffer); ++x)
        printf(" ");
      
      auto it2 = std::find_if(labels.begin(), labels.end(), [&](const pair<std::string, u16>& label){ return label.second + codeSegment.offset == pos; });
      
      if (it2 != labels.end())
        printf("<%s>  ", it2->first.c_str());
      
      auto it = std::find_if(jumps.begin(), jumps.end(), [&](const pair<u16, std::string>& jump){ return jump.first + codeSegment.offset == pos; });
      
      if (it != jumps.end())
        printf("%s   ", it->second.c_str());
      

    }
        
    printf("\n");
    
    pos += length;

  }
  
  u16 dataLen = dataSegment.length;
  for (int i = 0; i < dataLen / 8 + (dataLen % 8 != 0 ? 1 : 0); ++i)
  {
    printf("%04X: ", pos + i*8);
    
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
  
  std::list<Instruction>::iterator it = iterator();
  while (hasNext(it))
  {
    totalSize += it->getLength();
    ++it;
  }
  
  printf("  Code segment total size: %u bytes.\n", totalSize);
  
  if (entryPoint.isSet())
    printf("  Entry point specified at %.4Xh.\n", entryPoint.get());
  
  codeSegment.alloc(totalSize+codeSegment.offset);
  totalSize = 0;
  
  it = iterator();
  while (hasNext(it))
  {
    it->assemble(&codeSegment.data[totalSize+codeSegment.offset]);
    totalSize += it->getLength();
    ++it;
  }
}

void J80Assembler::solveDataReferences()
{
  printf("Solving data references.\n");
  
  for (auto &pair : dataReferences)
  {
    std::unordered_map<std::string, DataSegmentEntry>::iterator it = data.find(pair.second.label);
    
    if (it == data.end())
      printf("  Unresolved data label '%s'!", pair.second.label.c_str());
    else
    {
      if (pair.second.type == DataReference::Type::POINTER)
      {
        u16 address = it->second.offset + dataSegment.offset + pair.second.offset;
        printf("  > Data '%s' at address %.4Xh\n", pair.second.label.c_str(), address);
        
        codeSegment.data[pair.first+2] = address & 0xFF;
        codeSegment.data[pair.first+1] = (address >> 8) & 0xFF;
      }
      else if (pair.second.type == DataReference::Type::LENGTH8)
      {
        const DataSegmentEntry& entry = it->second;
        if (entry.length > 256)
          printf("Error! Length of '%s' is over 256 bytes", pair.second.label.c_str());
        else
          codeSegment.data[pair.first+2] = static_cast<u8>(entry.length);
      }
      else if (pair.second.type == DataReference::Type::LENGTH16)
      {
        u16 length = it->second.length;
        
        codeSegment.data[pair.first+2] = length & 0xFF;
        codeSegment.data[pair.first+1] = (length >> 8) & 0xFF;
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