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
  };
  
  class BaseType : public Type { };
  
  class RealType : public BaseType
  {
  
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
  public:
    Array(RealType* type) : type(std::unique_ptr<RealType>(type)) { }
    Array(const Array& other) : type(std::unique_ptr<RealType>(static_cast<RealType*>(other.type->copy()))) { }
    std::string mnemonic() const override { return type->mnemonic() + "[]"; }
    Array* copy() const override { return new Array(*this); }
  };
}




#endif