#ifndef __UI_H__
#define __UI_H__

#include <string>

#include <ncurses.h>
#include <panel.h>

#include "../utils.h"

class VM;

namespace vm
{
  class UI
  {
  private:
    WINDOW *wRegisters, *wStack, *wCode;
    PANEL *pRegs, *pStack, *pCode;
    bool shouldQuit;
    VM& vm;

    
  public:
    UI(VM& vm) : shouldQuit(false), vm(vm) { }
    
    void draw();
    void init();
    void shutdown();
    
    void handleEvents();
    bool shouldExit() { return shouldQuit; }
    
    void updateCode();
    void updateRegisters();
    void updateStack();


  };

}

#endif