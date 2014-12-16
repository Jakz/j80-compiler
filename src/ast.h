#ifndef __AST_H__
#define __AST_H__

#include <memory>
#include <vector>
#include <string>

#include "utils.h"

namespace nanoc
{

enum Type
{
  BYTE,
  WORD,
  BYTE_PTR,
  WORD_PTR,
  BYTE_ARRAY,
  WORD_ARRAY
};

enum Unary
{
  NOT,
  INCR,
  DECR,
  NEG
};


class ASTNode
{
  virtual void print() const = 0;
  
public:
  virtual void recursivePrint(u16 pad) const;
};




class ASTList : public ASTNode
{
private:
  std::vector<std::unique_ptr<ASTNode>> children;

  void print() const override { printf("List"); }
  
public:
  void recursivePrint(u16 pad) const override;
  
  void add(std::unique_ptr<ASTNode> item) { children.push_back(std::move(item)); }
};



class ASTDeclaration : public ASTNode
{
private:
  std::string name;
  
protected:
  ASTDeclaration(const std::string& name) : name(name) { }
  
  void print() const override { printf("Declaration(%s, %s)", name.c_str(), getTypeName().c_str()); }
  
public:
  virtual Type getType() const = 0;
  virtual std::string getTypeName() const = 0;
  
  static const char* nameForType(Type t)
  {
    switch (t) {
      case Type::BYTE: return "byte";
      case Type::WORD: return "word";
      case Type::BYTE_PTR: return "byte*";
      case Type::WORD_PTR: return "word*";
      case Type::BYTE_ARRAY: return "byte[]";
      case Type::WORD_ARRAY: return "word[]";
    }
  }
};

class ASTDeclarationByte : public ASTDeclaration
{
private:
  u8 value;
public:
  ASTDeclarationByte(const std::string& name, u8 value = 0) : ASTDeclaration(name), value(value) { }
  Type getType() const override { return Type::BYTE; }
  std::string getTypeName() const override  { return "byte"; }
};

class ASTDeclarationWord : public ASTDeclaration
{
private:
  u16 value;
public:
  ASTDeclarationWord(const std::string& name, u16 value = 0) : ASTDeclaration(name), value(value) { }
  Type getType() const override { return Type::WORD; }
  std::string getTypeName() const override  { return "word"; }
};

class ASTDeclarationPtr : public ASTDeclaration
{
private:
  u16 address;
  Type type;
public:
  ASTDeclarationPtr(const std::string& name, Type type, u16 address = 0) : ASTDeclaration(name), type(type), address(address) { }
  Type getType() const override { return type; }
  Type getItemType() const { return type == Type::WORD_PTR ? Type::WORD : Type::BYTE; }
  std::string getTypeName() const override  { return type == Type::WORD_PTR ? "word*" : "byte*"; }
};

class ASTDeclarationArray : public ASTDeclaration
{
private:
  u16 length;
  Type type;
public:
  ASTDeclarationArray(const std::string& name, Type type, u16 length = 0) : ASTDeclaration(name), type(type), length(length) { }
  Type getType() const override { return type; }
  Type getItemType() const { return type == Type::WORD_PTR ? Type::WORD : Type::BYTE; }
  std::string getTypeName() const override  { return  std::string(type == Type::WORD_PTR ? "word[" : "byte[")+std::to_string(length)+"]"; }
};



class ASTNumber : public ASTNode
{
private:
  u16 value;
  
public:
  ASTNumber(u16 value) : value(value) { }
  
  void print() const override {printf("Number(%d)", value); }
};








class ASTUnaryOperator : public ASTNode
{
private:
  Unary op;
  std::unique_ptr<ASTNode> child;
  
  virtual void print() const override { printf("UnaryOperator(%s)", nameForType(op)); }
  
public:
  ASTUnaryOperator(Unary op, std::unique_ptr<ASTNode> item) : op(op), child(std::move(item)) { }
  
  void recursivePrint(u16 pad) const override;
  
  
  static const char* nameForType(Unary op)
  {
    switch (op) {
      case Unary::NOT: return "!";
      case Unary::INCR: return "++";
      case Unary::DECR: return "--";
      case Unary::NEG: return "-";
      default: return "";
    }
  }
};


/*
 
 
 byte x;
 byte* x;

 word y;
 word* y;
 
 byte[10] x;
 word[15] x;
 
 string y = "antani"
 
*/

}

#endif