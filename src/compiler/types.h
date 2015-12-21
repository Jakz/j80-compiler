#ifndef __TYPES_H__
#define __TYPES_H__

#include "../utils.h"
#include "../format.h"

namespace nanoc {
  class SymbolTable;
  
  class Type
  {
  public:
    virtual std::string mnemonic() const = 0;
    virtual Type* copy() const = 0;
    virtual ~Type() { }
    virtual bool isVoid() const { return false; }
    virtual u16 getSize(const SymbolTable* table) const = 0;
    
    virtual bool isStruct(const SymbolTable *table) const { return false; }
  };
  
  class BaseType : public Type
  {
  public:
  };
  
  class RealType : public BaseType
  {
  public:
  };
  
  class Void : public BaseType
  {
  public:
    std::string mnemonic() const override { return "void"; }
    Void* copy() const override { return new Void(); }
    bool isVoid() const override { return true; }
    u16 getSize(const SymbolTable* table) const override { return 0; }
  };
  
  class Byte : public RealType
  {
  public:
    u16 getSize(const SymbolTable* table) const override { return 1; }
    std::string mnemonic() const override { return "byte"; }
    Byte* copy() const override { return new Byte(); }
  };
  
  class Word : public RealType
  {
  public:
    u16 getSize(const SymbolTable* table) const override { return 2; }
    std::string mnemonic() const override { return "word"; }
    Word* copy() const override { return new Word(); }

  };
  
  class Named : public RealType
  {
  private:
    std::string name;
  public:
    Named(const std::string& name) : name(name) { }
    u16 getSize(const SymbolTable* table) const override;
    std::string mnemonic() const override { return fmt::sprintf("Named(%s)", name.c_str()); }
    Named* copy() const override { return new Named(name); }
    bool isStruct(const SymbolTable *table) const override;
  };
  
  class Bool : public RealType
  {
  public:
    u16 getSize(const SymbolTable* table) const override { return 1; }
    std::string mnemonic() const override { return "bool"; }
    Bool* copy() const override { return new Bool(); }
  };
  
  class Pointer : public RealType
  {
  private:
    std::unique_ptr<BaseType> type;
  public:
    Pointer(BaseType* type) : type(std::unique_ptr<BaseType>(type)) { }
    Pointer(const Pointer& other) : type(std::unique_ptr<BaseType>(static_cast<BaseType*>(other.type->copy()))) { }
    
    u16 getSize(const SymbolTable* table) const override { return 2; }
    std::string mnemonic() const override { return type->mnemonic() + "*"; }
    Pointer* copy() const override { return new Pointer(*this); }
    
    Type* innerType() { return type.get(); }
  };
  
  class Array : public Type
  {
  private:
    std::unique_ptr<RealType> type;
    u16 length;
  public:
    Array(RealType* type, u16 length) : type(std::unique_ptr<RealType>(type)), length(length) { }
    Array(const Array& other) : type(std::unique_ptr<RealType>(static_cast<RealType*>(other.type->copy()))), length(other.length) { }
    u16 getSize(const SymbolTable* table) const override { return type->getSize(table)*length; }
    std::string mnemonic() const override { return type->mnemonic() + "["+std::to_string(length)+"]"; }
    Array* copy() const override { return new Array(*this); }
  };
}




#endif