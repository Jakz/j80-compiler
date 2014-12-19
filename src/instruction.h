#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

namespace Assembler
{
  #include "opcodes.h"
  
  enum InstructionLength : u8
  {
    LENGTH_0_BYTES = 0,
    LENGTH_1_BYTES = 1,
    LENGTH_2_BYTES = 2,
    LENGTH_3_BYTES = 3,
    LENGTH_4_BYTES = 4
  };
  
  class Instruction
  {
  public:
    u8 data[4];
    InstructionLength length;
    
  public:
    Instruction() : data{0}, length(LENGTH_2_BYTES) { }
    Instruction(InstructionLength length) : data{0}, length(length) { }
    
    virtual const InstructionLength getLength() const { return length; }
    
    virtual bool isReal() const { return true; }
    virtual std::string mnemonic() const { return std::string(); }
    
    virtual void assemble(u8* dest) const
    {
      memcpy(dest, &data, sizeof(u8)*length);
    }
  };
  
  class Label : public Instruction
  {
  private:
    std::string label;
    u16 address;
    bool solved;
    
  public:
    Label(const std::string& label) : Instruction(LENGTH_0_BYTES), label(label), address(0), solved(false) { }
    
    const std::string& getLabel() const { return label; }
    
    bool isReal() const override { return false; }
    
    bool mustBeSolved() { return !solved; }
    void solve(u16 address) { this->address = address; }
  };
  
  struct Address
  {
    enum Type
    {
      ABSOLUTE,
      LABEL
    } type;
    
    u16 address;
    std::string label;
    
    Address(u16 address) : type(ABSOLUTE), address(address) { }
    Address(const std::string& label) : type(LABEL), label(label) { }
  };
  
  struct Value8
  {
    enum Type
    {
      VALUE,
      DATA_LENGTH
    } type;
    
    u8 value;
    std::string label;
    
    Value8(u8 value) : type(VALUE), value(value) { }
    Value8(const std::string& label) : type(DATA_LENGTH), label(label) { }
  };
  
  class InstructionLD_NN : public Instruction
  {
  private:
    Reg dst;
    Value8 value;
    
  public:
    InstructionLD_NN(Reg dst, u8 value) : dst(dst), value(Value8(value)) { }
    InstructionLD_NN(Reg dst, const std::string& label) : dst(dst), value(Value8(label)) { }

    
    std::string mnemonic() const { return fmt::sprintf("LD %s, %.2Xh", Opcodes::reg8(dst), value.value); }
    
    void assemble(u8* dest) const override
    {
      dest[0] = (OPCODE_LD_NN << 3) | dst;
      dest[1] = ALU_TRANSFER_B8;
      dest[2] = value.value;
    }
    
    const std::string& getLabel() { return value.label; }
    bool mustBeSolved() { return value.type != Value8::Type::VALUE; }
    void solve(u8 value) { this->value.value = value; this->value.type = Value8::Type::VALUE; }
  };
  
  class InstructionAddressable : public Instruction
  {
  protected:
    Address address;
    
  public:
    InstructionAddressable(InstructionLength length, Address address) : Instruction(length), address(address) { }
    
    const std::string& getLabel() { return address.label; }
    bool mustBeSolved() { return address.type == Address::Type::LABEL; }
    void solve(u16 address) { this->address.address = address; this->address.type = Address::Type::ABSOLUTE; }
  };
  
  class InstructionJMP_NNNN : public InstructionAddressable
  {
  private:
    JumpCondition condition;
    
  public:
    InstructionJMP_NNNN(JumpCondition condition, u16 address) : InstructionAddressable(LENGTH_3_BYTES, Address(address)), condition(condition) { }
    InstructionJMP_NNNN(JumpCondition condition, const std::string& label) : InstructionAddressable(LENGTH_3_BYTES, Address(label)), condition(condition) { }
    
    std::string mnemonic() const override {
      if (address.label.empty())
        return fmt::sprintf("JMP%s %.4Xh", Opcodes::condName(condition), address.address);
      else
        return fmt::sprintf("JMP%s %.4Xh (%s)", Opcodes::condName(condition), address.address, address.label.c_str());
    }
    
    void assemble(u8* dest) const override
    {
      dest[0] = (OPCODE_JMPC_NNNN << 3) | condition;
      dest[1] = (address.address >> 8) & 0xFF;
      dest[2] = address.address & 0xFF;
    }
  };
  
  class InstructionCALL_NNNN : public InstructionAddressable
  {
  private:
    JumpCondition condition;
    
  public:
    InstructionCALL_NNNN(JumpCondition condition, u16 address) : InstructionAddressable(LENGTH_3_BYTES, Address(address)), condition(condition) { }
    InstructionCALL_NNNN(JumpCondition condition, const std::string& label) : InstructionAddressable(LENGTH_3_BYTES, Address(label)), condition(condition) { }
    
    std::string mnemonic() const override {
      if (address.label.empty())
        return fmt::sprintf("CALL%s %.4Xh", Opcodes::condName(condition), address.address);
      else
        return fmt::sprintf("CALL%s %.4Xh (%s)", Opcodes::condName(condition), address.address, address.label.c_str());
    }
    
    void assemble(u8* dest) const override
    {
      dest[0] = (OPCODE_CALLC << 3) | condition;
      dest[1] = (address.address >> 8) & 0xFF;
      dest[2] = address.address & 0xFF;
    }
  };
  
  class InstructionRET : public Instruction
  {
  private:
    JumpCondition condition;
    
  public:
    InstructionRET(JumpCondition condition) : Instruction(LENGTH_1_BYTES), condition(condition) { }
    
    std::string mnemonic() const { return fmt::sprintf("RET%s", Opcodes::condName(condition)); }
    void assemble(u8* dest) const override { dest[0] = (OPCODE_RETC << 3) | condition;; }
  };
  
  class InstructionNOP : public Instruction
  {
  public:
    InstructionNOP() : Instruction(LENGTH_1_BYTES) { }
    
    std::string mnemonic() const { return "NOP"; }
    void assemble(u8* dest) const override { dest[0] = OPCODE_NOP; }
  };
}

#endif
