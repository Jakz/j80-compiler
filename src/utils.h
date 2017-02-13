#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define GB_CPU

#define USE_SDL

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef uint64_t u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

#define KB2 (2048)
#define KB4 (4096)
#define KB8 (8192)
#define KB16 (16384)
#define KB32 (32768)

#define KB131 (131072)

#define MB4 (4194304)

namespace nanoc {
  
  enum Unary
  {
    NOT,
    NEG,
    FLIP,
    ADDRESSOF,
    DEREFERENCE
  };
  
  enum Binary
  {
    ADDITION,
    SUBTRACTION,
    AND,
    OR,
    XOR,
    
    EQ,
    NEQ,
    GREATEREQ,
    LESSEQ,
    GREATER,
    LESS,
    
    LOR,
    LAND
  };
  
  enum Ternary
  {
    ELVIS
  };
  
  class Mnemonics
  {
  public:
    static std::string mnemonicForUnary(Unary op);
    static std::string mnemonicForBinary(Binary op);
    static std::string mnemonicForTernary(Ternary op);
  };
}

template<typename T, typename V> bool valueFitsType(V v)
{
  static_assert(sizeof(T) < sizeof(V), "");
  using base_t = typename std::make_signed<V>::type;
  using signed_t = typename std::make_signed<T>::type;
  using unsigned_t = typename std::make_unsigned<T>::type;
  
  bool unfitAsSigned = ((base_t)v < 0 && (base_t)v != (signed_t)v);
  bool unfitAsUnsigned = ((base_t)v > 0 && v != (unsigned_t)v);
  
  return !(unfitAsSigned || unfitAsUnsigned);
}

struct Result
{
  std::string message;
  bool success;
  
  Result() : success(true) { }
  Result(const std::string& message) : success(false), message(message) { }
  operator bool() const { return success; }
};

class Utils
{
  private:
    static int fd;
    static fpos_t pos;
  
  public:
    static void switchStdout(const char *newStream)
    {
      fflush(stdout);
      fgetpos(stdout, &pos);
      fd = dup(fileno(stdout));
      freopen(newStream, "w", stdout);
    }
    
    static void revertStdout()
    {
      fflush(stdout);
      dup2(fd, fileno(stdout));
      close(fd);
      clearerr(stdout);
      fsetpos(stdout, &pos);
    }
  
    static void switchStdin(const char *newStream)
    {
      fflush(stdin);
      fgetpos(stdin, &pos);
      fd = dup(fileno(stdin));
      freopen(newStream, "r", stdin);
    }
    
    static void revertStdin()
    {
      fflush(stdin);
      dup2(fd, fileno(stdin));
      close(fd);
      clearerr(stdin);
      fsetpos(stdin, &pos);
    }
  
    static long fileLength(FILE *file)
    {
      long cur = ftell(file);
      
      fseek(file, 0, SEEK_END);
      
      long length = ftell(file);
      
      fseek(file, cur, SEEK_SET);
      
      return length;
    }
  
    static std::string execute(std::string command);
  
    static inline bool bit(u8 value, u8 bit)
    {
      return value & (1 << bit);
    }
  
    static inline u8 set(u8 value, u8 bit)
    {
      return value | (1 << bit);
    }
  
    static inline u8 res(u8 value, u8 bit)
    {
      return value & ~(1 << bit);
    }
  
    /*static void assert(bool condition, const char* type, const char* message)
    {
      if (!condition)
        printf("[%s] ERROR: %s", type, message);
    }*/
};


#endif
