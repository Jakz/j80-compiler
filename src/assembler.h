#ifndef _GB_ASSEMBLER_H_
#define _GB_ASSEMBLER_H_


#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>

#include "assembler/j80lexer.h"
#include "assembler/j80parser.hpp"
#include "assembler/location.h"

#include "format.h"

#include "instruction.h"
#include "opcodes.h"

enum class Log
{
  ERROR,
  WARNING,
  INFO,
  VERBOSE_INFO
};

namespace Assembler
{
  
  template<typename T>
  struct Optional
  {
  private:
    T value;
    bool hasBeenSet;
    
  public:
    Optional() : hasBeenSet(false) { }
    Optional(T value) : value(value), hasBeenSet(true) { }
    bool set(T value) {
      if (hasBeenSet) return false;
      
      this->value = value;
      hasBeenSet = true;
      return true;
    }
    bool isSet() const { return hasBeenSet; }
    T get() const { return value; }
  };
  
  class J80Assembler
  {
  private:
    const char* sprintf(const char* fmt, ...) const;
    
    u16 position;
    std::list<std::unique_ptr<Instruction>> instructions;
    
    std::vector<std::pair<u16, DataReference> > dataReferences;
    
    data_map data;
    std::unordered_map<std::string, u16> consts;
    
    bool irqs[4] = {false,false,false,false};
    
    Optional<u16> stackBase;
    Optional<u16> entryPoint;
    
    DataSegment dataSegment;
    CodeSegment codeSegment;
    
    //static Instruction *build(InstructionLength len) { return new Instruction(len, position); }
    //static void insert(Instruction *i) { position += i->length; instructions.push_back(i); }
    
  public:
    J80Assembler();
    
    void log(Log l, bool newline, const char* str, ...) const;
    
    std::string file;
    
    void error (const Assembler::location& l, const std::string& m);
    void error (const std::string& m);
    
    bool parse(const std::string& filename);
    
    //static u16 size();
    
    const u16 maxNumberOfInterrupts() const { return 4; }
    bool isInterruptAvailable(u8 index) const { return !irqs[index]; }
    void markInterrupt(u8 index) { irqs[index] = true; }
    
    bool setStackBase(u16 address) { return stackBase.set(address); }
    bool setEntryPoint(u16 address) { return entryPoint.set(address); }
    
    Instruction* preamble(InstructionLength len)
    {
      return new Instruction(len);
    }
    
    std::unique_ptr<Instruction>& postamble(Instruction *i)
    {
      position += i->getLength();
      instructions.push_back(std::unique_ptr<Instruction>(i));
      return instructions.back();
    }
    
    void add(Instruction* i)
    {
      position += i->getLength();
      instructions.push_back(std::unique_ptr<Instruction>(i));
    }
    
    void assembleLD_PTR_NNNN(Reg dst, u16 address, const std::string& label = std::string(), s8 offset = 0)
    {
      if (!label.empty())
        dataReferences.push_back(std::make_pair(position, DataReference(label,offset)));
      
      Instruction* i = preamble(LENGTH_3_BYTES);
      i->data[0] = (OPCODE_LD_PTR_NNNN << 3) | dst;
      i->data[2] = address & 0xFF;
      i->data[1] = (address >> 8) & 0xFF;
      postamble(i);
    }
    
    void assembleLD_PTR_PP(Reg dst, Reg src, s8 value)
    {
      Instruction* i = preamble(LENGTH_3_BYTES);
      i->data[0] = (OPCODE_LD_PTR_PP << 3) | dst;
      i->data[1] = (src << 5) | ALU_ADD_NO_FLAGS;
      i->data[2] = value;
      postamble(i);
    }
    
    void assembleSD_PTR_NNNN(Reg src, u16 address, const std::string& label = std::string(), s8 offset = 0)
    {
      if (!label.empty())
        dataReferences.push_back(std::make_pair(position, DataReference(label,offset)));
      
      Instruction* i = preamble(LENGTH_3_BYTES);
      i->data[0] = (OPCODE_SD_PTR_NNNN << 3) | src;
      i->data[2] = address & 0xFF;
      i->data[1] = (address >> 8) & 0xFF;
      postamble(i);
    }
    
    void assembleSD_PTR_PP(Reg src, Reg raddr, s8 value)
    {
      Instruction* i = preamble(LENGTH_3_BYTES);
      i->data[0] = (OPCODE_SD_PTR_PP << 3) | src;
      i->data[1] = (raddr << 5) | ALU_ADD_NO_FLAGS;
      i->data[2] = value;
      postamble(i);
    }
    
    void assembleALU_REG(Reg dst, Reg src1, Reg src2, AluOp opcode, bool extended)
    {
      Instruction* i = preamble(LENGTH_3_BYTES);
      if (extended) opcode = static_cast<AluOp>(opcode | 0b1);
      
      i->data[0] = (OPCODE_ALU_REG << 3) | dst;
      i->data[1] = (src1 << 5) | opcode;
      i->data[2] = (src2 << 5);
      postamble(i);
    }
    
    void assembleALU_NN(Reg dst, Reg src1, AluOp opcode, u8 value)
    {
      Instruction* i = preamble(LENGTH_3_BYTES);
      
      i->data[0] = (OPCODE_ALU_NN << 3) | dst;
      i->data[1] = (src1 << 5) | opcode;
      i->data[2] = value;
      postamble(i);
    }
    
    void assembleALU_NNNN(Reg dst, Reg src1, AluOp opcode, u16 value)
    {
      Instruction* i = preamble(LENGTH_4_BYTES);
      opcode = static_cast<AluOp>(opcode | 0b1);
      
      i->data[0] = (OPCODE_ALU_NNNN << 3) | dst;
      i->data[1] = (src1 << 5) | opcode;
      i->data[2] = value & 0xFF;
      i->data[3] = (value >> 8) & 0xFF;
      postamble(i);
    }
    
    void assembleJMP_PP(JumpCondition cond, Reg reg)
    {
      Instruction* i = preamble(LENGTH_2_BYTES);
      
      i->data[0] = (OPCODE_JMPC_PP << 3) | cond;
      i->data[1] = (reg << 5) | ALU_TRANSFER_B16;
      
      postamble(i);
    }
    
    // CHEATED LENGTHS
    /*
     PUSH, POP, RET, LF, SF, EI, DI, INT
     
     
     */
    
    void assembleShort(Opcode opcode)
    {
      Instruction* i = preamble(LENGTH_1_BYTES);
      i->data[0] = opcode << 3;
      postamble(i);
    }
    
    void assembleShortWithReg(Opcode opcode, Reg reg)
    {
      Instruction* i = preamble(LENGTH_1_BYTES);
      i->data[0] = (opcode << 3) | reg;
      postamble(i);
    }
    
    void assembleRET(JumpCondition cond)
    {
      Instruction* i = preamble(LENGTH_1_BYTES);
      i->data[0] = (OPCODE_RETC << 3) | cond;
      postamble(i);
    }
    
    void assembleCMP_REG(Reg dst, Reg src1, bool extended)
    {
      Instruction *i = preamble(LENGTH_2_BYTES);
      AluOp opcode = ALU_SUB8;
      if (extended) opcode = ALU_SUB16;
      
      i->data[0] = (OPCODE_CMP_REG << 3) | dst;
      i->data[1] = (src1 << 5) | opcode;
      postamble(i);
    }
    
    void addData(const std::string& label, const DataSegmentEntry& entry)
    {
      data.lru.push_back(data.map.insert(std::make_pair(label, entry)).first);
    }

    void addConstValue(const std::string& label, u16 value)
    {
      consts[label] = value;
    }
    
    void prepareSource();
    
    void buildDataSegment();
    void buildCodeSegment();
    Result solveDataReferences();
    Result solveJumps();
    
    u16 computeDataSegmentOffset()
    {
      u16 offset = 0;
      
      for (const auto &i : instructions)
        offset += i->getLength();
      
      return offset;
    }
    
    Result assemble()
    {
      Result result;
      
      if (entryPoint.isSet())
        codeSegment.offset = entryPoint.get();
      
      prepareSource();
      
      buildDataSegment();
      
      if (result)
        result = solveDataReferences();
      
      if (result)
        result = solveJumps();
      buildCodeSegment();
      dataSegment.offset = codeSegment.length + codeSegment.offset;
      
      return result;
    }
    
    const DataSegment& getDataSegment() { return dataSegment; }
    const CodeSegment& getCodeSegment() { return codeSegment; }
    
    std::list<std::unique_ptr<Instruction>>::const_iterator iterator() { return instructions.begin(); }
    bool hasNext(std::list<std::unique_ptr<Instruction>>::const_iterator it) { return it  != instructions.end(); }
    
    void printProgram(std::ostream& out) const;
    void saveForLogisim(const std::string& filename) const;
    void saveBinary(const std::string& filename) const;
  };
  
}

#endif
