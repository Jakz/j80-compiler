#ifndef __UI_H__
#define __UI_H__

#include <string>

#include <ncurses.h>
#include <panel.h>

#include "../utils.h"

class VM;
class StdOut;

namespace vm
{
  class UI
  {
  private:
    WINDOW *wRegisters, *wStack, *wCode, *wConsole;
    PANEL *pRegs, *pStack, *pCode, *pConsole;
    bool shouldQuit;
    VM& vm;
    
    u64 counter;
    u32 stepSize;

    
  public:
    UI(VM& vm) : shouldQuit(false), vm(vm), counter(0), stepSize(256) { }
    
    void draw();
    void init();
    void shutdown();
    
    void handleEvents();
    bool shouldExit() { return shouldQuit; }
    
    void updateCode();
    void updateRegisters();
    void updateStack();
    void updateConsole();
    
    StdOut* getStdOut();
    


  };

}

#endif