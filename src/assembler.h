#ifndef _GB_ASSEMBLER_H_
#define _GB_ASSEMBLER_H_


#include "opcodes.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>

#include "assembler/j80lexer.h"
#include "assembler/j80parser.hpp"
#include "assembler/location.hh"

namespace Assembler
{

struct BinaryCode
{
  u8 *code;
  u32 length;
};

enum InstructionLength : u8
{
  LENGTH_1_BYTES = 1,
  LENGTH_2_BYTES = 2,
  LENGTH_3_BYTES = 3,
  LENGTH_4_BYTES = 4
};
  
#define DATA_OFFSET (0x1000)

struct Instruction
{
  u8 data[4];
  InstructionLength length;
  u16 position;
  
  Instruction() : data{0}, length(LENGTH_2_BYTES), position(0) { }
  Instruction(InstructionLength length, u16 position) : data{0}, length(length), position(position) { }
};
  
struct DataSegmentEntry
{
  u8 *data;
  u16 length;
  u16 offset;
  
  DataSegmentEntry() : data(nullptr), length(0), offset(0) { }
  DataSegmentEntry(const std::string& ascii) { offset = 0x0000; data = reinterpret_cast<u8*>(strdup(ascii.c_str())); length = ascii.length(); }
  DataSegmentEntry(u16 size) { offset = 0x0000; data = new u8[size]; memset(data, 0, size); length = size; }
  ~DataSegmentEntry() { /* TODO: can't release because it's copy constructed by STL*/ }
};
  
class DataSegment
{
  public:
    u16 offset;
    u8 *data;
    u16 length;

    DataSegment() : offset(0), data(nullptr), length(0) { }
    void alloc(u16 length) { if (data) delete[] data; data = new u8[length]; this->length = length;}
    ~DataSegment() { delete [] data; }
};
  
class CodeSegment
{
  public:
    u16 offset;
    u8 *data;
    u16 length;
  
  CodeSegment() : offset(0), data(nullptr), length(0) { }
  void alloc(u16 length) { delete[] data; data = new u8[length]; this->length = length;}
  ~CodeSegment() { delete [] data; }
};
  
struct DataReference
{
  enum class Type
  {
    POINTER,
    LENGTH8,
    LENGTH16
  };
  
  Type type;
  std::string label;
  s8 offset;
  
  DataReference(const std::string& label, s8 offset) : type(Type::POINTER), label(label), offset(offset) { }
  DataReference(const std::string& label, Type type) : type(type), label(label), offset(0) { }
};


BinaryCode assemble(const char*);
BinaryCode assembleLine(const char *);

class J80Assembler
{
  private:
    u16 position;
    std::list<Instruction> instructions;
    std::unordered_map<std::string,u16> labels;
  
    std::vector<std::pair<u16, std::string> > jumps;
    std::vector<std::pair<u16, DataReference> > dataReferences;
  
    std::unordered_map<std::string, DataSegmentEntry> data;
  
    s8 currentIrq;
    std::list<Instruction> irqs[4];
  
    //static Instruction *build(InstructionLength len) { return new Instruction(len, position); }
    //static void insert(Instruction *i) { position += i->length; instructions.push_back(i); }
  
  public:
    J80Assembler();
  
    DataSegment dataSegment;
    CodeSegment codeSegment;
  
    std::string file;
  
    void error (const Assembler::location& l, const std::string& m);
    void error (const std::string& m);
  
    bool parse(const std::string& filename);
  
    void assemble(int opcode, int opcode2 = -1, int opcode3 = -1);
    //static u16 size();
    BinaryCode consolidate();

    Instruction preamble(InstructionLength len)
    {
      return Instruction(len, position);
    }
  
    void postamble(Instruction &i)
    {
      position += i.length;
      instructions.push_back(i);
    }
  
    void assembleLD_RSH_LSH(Reg dst, Reg src, AluOp opcode, bool extended)
    {
      Instruction i = preamble(LENGTH_2_BYTES);
      if (extended) opcode = static_cast<AluOp>(opcode | 0b1);
      i.data[0] = (OPCODE_LD_RSH_LSH << 3) | dst;
      i.data[1] = (src << 5) | opcode;
      postamble(i);
    }
  
    void assembleLD_NN(Reg dst, u8 value, const std::string& label = std::string())
    {
      //TODO: add reference for length of data label
      if (!label.empty())
        dataReferences.push_back(std::make_pair(position, DataReference(label, DataReference::Type::LENGTH8)));
      
      Instruction i = preamble(LENGTH_3_BYTES);
      i.data[0] = (OPCODE_LD_NN << 3) | dst;
      i.data[1] = ALU_TRANSFER_B8;
      i.data[2] = value;
      postamble(i);
    }
  
    void assembleLD_NNNN(Reg dst, u16 value, const std::string& label = std::string(), s8 offset = 0)
    {
      if (!label.empty())
        dataReferences.push_back(std::make_pair(position, DataReference(label,offset)));
      
      Instruction i = preamble(LENGTH_3_BYTES);
      printf("value: %.4X\n",value);
      i.data[0] = (OPCODE_LD_NNNN << 3) | dst;
      i.data[2] = value & 0xFF;
      i.data[1] = (value >> 8) & 0xFF;
      postamble(i);
    }
  
    void assembleLD_PTR_NNNN(Reg dst, u16 address, const std::string& label = std::string(), s8 offset = 0)
    {
      if (!label.empty())
        dataReferences.push_back(std::make_pair(position, DataReference(label,offset)));
      
      Instruction i = preamble(LENGTH_3_BYTES);
      i.data[0] = (OPCODE_LD_PTR_NNNN << 3) | dst;
      i.data[2] = address & 0xFF;
      i.data[1] = (address >> 8) & 0xFF;
      postamble(i);
    }
  
    void assembleLD_PTR_PP(Reg dst, Reg src, s8 value)
    {
      Instruction i = preamble(LENGTH_3_BYTES);
      i.data[0] = (OPCODE_LD_PTR_PP << 3) | dst;
      i.data[1] = (src << 5) | ALU_ADD_NO_FLAGS;
      i.data[2] = value;
      postamble(i);
    }
  
    void assembleSD_PTR_NNNN(Reg src, u16 address, const std::string& label = std::string(), s8 offset = 0)
    {
      if (!label.empty())
        dataReferences.push_back(std::make_pair(position, DataReference(label,offset)));
      
      Instruction i = preamble(LENGTH_3_BYTES);
      i.data[0] = (OPCODE_SD_PTR_NNNN << 3) | src;
      i.data[2] = address & 0xFF;
      i.data[1] = (address >> 8) & 0xFF;
      postamble(i);
    }
  
    void assembleSD_PTR_PP(Reg src, Reg raddr, s8 value)
    {
      Instruction i = preamble(LENGTH_3_BYTES);
      i.data[0] = (OPCODE_SD_PTR_PP << 3) | src;
      i.data[1] = (raddr << 5) | ALU_ADD_NO_FLAGS;
      i.data[2] = value;
      postamble(i);
    }
  
    void assembleALU_REG(Reg dst, Reg src1, Reg src2, AluOp opcode, bool extended)
    {
      Instruction i = preamble(LENGTH_3_BYTES);
      if (extended) opcode = static_cast<AluOp>(opcode | 0b1);
      
      i.data[0] = (OPCODE_ALU_REG << 3) | dst;
      i.data[1] = (src1 << 5) | opcode;
      i.data[2] = (src2 << 5);
      postamble(i);
    }
  
    void assembleALU_NN(Reg dst, Reg src1, AluOp opcode, u8 value)
    {
      Instruction i = preamble(LENGTH_3_BYTES);
      
      i.data[0] = (OPCODE_ALU_NN << 3) | dst;
      i.data[1] = (src1 << 5) | opcode;
      i.data[2] = value;
      postamble(i);
    }
  
    void assembleALU_NNNN(Reg dst, Reg src1, AluOp opcode, u16 value)
    {
      Instruction i = preamble(LENGTH_4_BYTES);
      opcode = static_cast<AluOp>(opcode | 0b1);
      
      i.data[0] = (OPCODE_ALU_NNNN << 3) | dst;
      i.data[1] = (src1 << 5) | opcode;
      i.data[2] = value & 0xFF;
      i.data[3] = (value >> 8) & 0xFF;
      postamble(i);
    }
  
    void assembleJMP_NNNN(JumpCondition cond, const std::string& label)
    {
      jumps.push_back(std::make_pair(position, label));
      
      Instruction i = preamble(LENGTH_3_BYTES);

      i.data[0] = (OPCODE_JMPC_NNNN << 3) | cond;
      /*i.data[1] = address & 0xFF;
      i.data[2] = (address >> 8) & 0xFF;*/
      
      postamble(i);
    }
  
    void assembleJMP_NNNN(JumpCondition cond, const u16 address)
    {
      Instruction i = preamble(LENGTH_3_BYTES);
      
      i.data[0] = (OPCODE_JMPC_NNNN << 3) | cond;
      i.data[2] = address & 0xFF;
      i.data[1] = (address >> 8) & 0xFF;
      postamble(i);
    }
  
    void assembleJMP_PP(JumpCondition cond, Reg reg)
    {
      Instruction i = preamble(LENGTH_2_BYTES);
      
      i.data[0] = (OPCODE_JMPC_PP << 3) | cond;
      i.data[1] = (reg << 5) | ALU_TRANSFER_B16;
      
      postamble(i);
    }
    
    // CHEATED LENGTHS
    /*
     PUSH, POP, RET, LF, SF, EI, DI, INT
     
     
     */
  
    void assembleShort(Opcode opcode)
    {
      Instruction i = preamble(LENGTH_1_BYTES);
      i.data[0] = opcode << 3;
      postamble(i);
    }
  
    void assembleShortWithReg(Opcode opcode, Reg reg)
    {
      Instruction i = preamble(LENGTH_1_BYTES);
      i.data[0] = (opcode << 3) | reg;
      postamble(i);
    }
  
    void assembleRET(JumpCondition cond)
    {
      Instruction i = preamble(LENGTH_1_BYTES);
      i.data[0] = (OPCODE_RETC << 3) | cond;
      postamble(i);
    }
  
    void assembleCALL_NNNN(JumpCondition cond, const std::string& label)
    {
      jumps.push_back(std::make_pair(position, label));

      
      Instruction i = preamble(LENGTH_3_BYTES);
      
      i.data[0] = (OPCODE_CALLC << 3) | cond;
      /*i.data[1] = address & 0xFF;
      i.data[2] = (address >> 8) & 0xFF;*/
      
      postamble(i);
    }
  
    void assembleCALL_NNNN(JumpCondition cond, const u16 address)
    {
      Instruction i = preamble(LENGTH_3_BYTES);
      
      i.data[0] = (OPCODE_CALLC << 3) | cond;
      i.data[2] = address & 0xFF;
      i.data[1] = (address >> 8) & 0xFF;
      
      postamble(i);
    }
  
    void assembleCMP_REG(Reg dst, Reg src1, bool extended)
    {
      Instruction i = preamble(LENGTH_2_BYTES);
      AluOp opcode = ALU_SUB8;
      if (extended) opcode = ALU_SUB16;
      
      i.data[0] = (OPCODE_CMP_REG << 3) | dst;
      i.data[1] = (src1 << 5) | opcode;
      postamble(i);
    }
    
    void assembleCMP_NN(Reg dst, u8 value)
    {
      Instruction i = preamble(LENGTH_3_BYTES);
      i.data[0] = (OPCODE_CMP_NN << 3) | dst;
      i.data[1] = ALU_SUB8;
      i.data[2] = value;
      postamble(i);
    }
    
    void assembleCMP_NNNN(Reg dst, u16 value)
    {
      Instruction i = preamble(LENGTH_4_BYTES);      
      i.data[0] = (OPCODE_CMP_NNNN << 3) | dst;
      i.data[1] = ALU_SUB16;
      i.data[2] = value & 0xFF;
      i.data[3] = (value >> 8) & 0xFF;
      postamble(i);
    }
  
  
    bool solveJumps();
  
  
    void addAsciiData(const std::string& label, const std::string& sdata)
    {
      data[label] = DataSegmentEntry(sdata);
    }
  
    void addEmptyData(const std::string& label, u16 size)
    {
      data[label] = DataSegmentEntry(size);
    }
  
    void buildDataSegment()
    {
      printf("Building data segment.\n");
      u16 totalSize = 0;
      for (auto &entry : data)
        totalSize += entry.second.length;
      
      printf("Data segment size: %u\n", totalSize);
      
      dataSegment.alloc(totalSize);
      
      totalSize = 0;
      
      for (auto &entry : data)
      {
        memcpy(&dataSegment.data[totalSize], entry.second.data, entry.second.length);
        entry.second.offset = totalSize;
        
        printf("Data named %s at offset %.4X\n",entry.first.c_str(),entry.second.offset);
        
        totalSize += entry.second.length;
      }
    }
  
    void buildCodeSegment()
    {
      printf("Building code segment.\n");

      u16 totalSize = 0;
      
      std::list<Instruction>::iterator it = iterator();
      while (hasNext(it))
      {
        totalSize += it->length;
        ++it;
      }
      
      printf("Code segment total size: %u\n", totalSize);
      
      codeSegment.alloc(totalSize);
      totalSize = 0;
      
      it = iterator();
      while (hasNext(it))
      {
        memcpy(&codeSegment.data[totalSize], it->data, it->length);
        totalSize += it->length;
        ++it;
      }
    }
  
    void solveDataReferences()
    {
      printf("Solving data references.\n");
      
      for (auto &pair : dataReferences)
      {
        std::unordered_map<std::string, DataSegmentEntry>::iterator it = data.find(pair.second.label);
        
        if (it == data.end())
          printf("Unresolved data label '%s'!", pair.second.label.c_str());
        else
        {
          if (pair.second.type == DataReference::Type::POINTER)
          {
            u16 address = it->second.offset + dataSegment.offset + pair.second.offset;
            printf("Solving data label '%s' at address %.4X\n", pair.second.label.c_str(), address);

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
  
    void assemble()
    {
      buildCodeSegment();
      codeSegment.offset = 0;
      buildDataSegment();
      dataSegment.offset = codeSegment.length;
      solveJumps();
      solveDataReferences();
    }
  
    void interruptStart(u8 index) { currentIrq = index; }
    void interruptEnd() { currentIrq = -1; }
  
    void placeLabel(const std::string& label);
  
    std::list<Instruction>::iterator iterator() { return instructions.begin(); }
    bool hasNext(std::list<Instruction>::iterator it) { return it  != instructions.end(); }

    void printProgram() const;
    void saveForLogisim(const std::string& filename) const;
    void saveBinary(const std::string& filename) const;
  
    constexpr static const BinaryCode INVALID = BinaryCode{nullptr, 0};
  
};
  
}

#endif