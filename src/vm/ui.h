#ifndef __UI_H__
#define __UI_H__

#include <string>

#include <ncurses.h>
#include <panel.h>

namespace vm
{

  enum class DataWidth : u8
  {
    BYTE,
    WORD,
    DWORD,
  };

  struct CpuSpec
  {
    DataWidth widthAddress;
    DataWidth widthData;
    DataWidth widthMaxInstruction;
    std::string name;
  };

  class UI
  {
  private:
    WINDOW *wRegisters, *wRom, *wRam, *wOpt, *wPorts;
    PANEL *pRegs, *pRom, *pRam, *pOpt, *pPorts; 

  public:
    UI(const CpuSpec& spec);
  
    void init();


  };

}


#endif