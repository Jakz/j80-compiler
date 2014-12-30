#ifndef __TYPES_H__
#define __TYPES_H__

#include "../utils.h"
#include "../format.h"

namespace nanoc {
  class Type
  {
  public:
    virtual std::string mnemonic() const = 0;
    virtual Type* copy() const = 0;
    virtual ~Type() { }
  };
  
  class BaseType : public Type { };
  
  class RealType : public BaseType
  {
  public:
    u16 getSize() const { return 1; }
  };
  
  class Void : public BaseType
  {
  public:
    std::string mnemonic() const override { return "void"; }
    Void* copy() const override { return new Void(); }
  };
  
  class Byte : public RealType
  {
  public:
    std::string mnemonic() const override { return "byte"; }
    Byte* copy() const override { return new Byte(); }
  };
  
  class Word : public RealType
  {
  public:
    std::string mnemonic() const override { return "word"; }
    Word* copy() const override { return new Word(); }

  };
  
  class Named : public RealType
  {
  private:
    std::string name;
  public:
    Named(const std::string& name) : name(name) { }
    std::string mnemonic() const override { return fmt::sprintf("Named(%s)", name.c_str()); }
    Named* copy() const override { return new Named(name); }
  };
  
  class Bool : public RealType
  {
  public:
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
    std::string mnemonic() const override { return type->mnemonic() + "["+std::to_string(length)+"]"; }
    Array* copy() const override { return new Array(*this); }
  };
}




#endif