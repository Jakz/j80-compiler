#include "ui.h"

#include "vm.h"
#include "opcodes.h"

#include <ncurses.h>
#include <panel.h>
#include <sys/ioctl.h>
#include <unistd.h>

using namespace vm;

static int width, height;

constexpr u32 WINDOW_REGS_HEIGHT = 8;
constexpr u32 SIDE_PANEL_WIDTH = 33;
constexpr u32 LOWER_PANEL_HEIGHT = 5;

void UI::init()
{
  system("clear");
  
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  
  height = w.ws_row;
  width = w.ws_col;
  
  initscr();
  keypad(stdscr, true);
  noecho();
  cbreak();
  curs_set(0);
  
  wRegisters = newwin(WINDOW_REGS_HEIGHT, SIDE_PANEL_WIDTH, 0, width-SIDE_PANEL_WIDTH);
  wStack = newwin(height-WINDOW_REGS_HEIGHT, SIDE_PANEL_WIDTH, WINDOW_REGS_HEIGHT, width-SIDE_PANEL_WIDTH);
  wCode = newwin(height-LOWER_PANEL_HEIGHT, width-SIDE_PANEL_WIDTH, 0, 0);
  wConsole = newwin(LOWER_PANEL_HEIGHT, width-SIDE_PANEL_WIDTH, height-LOWER_PANEL_HEIGHT, 0);
  
  pRegs = new_panel(wRegisters);
  pStack = new_panel(wStack);
  pCode = new_panel(wCode);
  pConsole = new_panel(wConsole);
  
}

void UI::draw()
{
  updateCode();
  updateRegisters();
  updateStack();
  updateConsole();
  update_panels();
	doupdate();
}



void UI::updateRegisters()
{
  wclear(wRegisters);
  box(wRegisters, 0, 0);
  mvwprintw(wRegisters, 0, 1, "[Registers]");
  
  mvwprintw(wRegisters, 1, 2, "BA: %04Xh", vm.reg16(REG_BA));
  mvwprintw(wRegisters, 2, 2, "CD: %04Xh", vm.reg16(REG_CD));
  mvwprintw(wRegisters, 3, 2, "EF: %04Xh", vm.reg16(REG_EF));
  mvwprintw(wRegisters, 4, 2, "XY: %04Xh", vm.reg16(REG_XY));
  
  mvwprintw(wRegisters, 1, 2+14, "SP: %04Xh", vm.reg16(REG_SP));
  mvwprintw(wRegisters, 2, 2+14, "FP: %04Xh", vm.reg16(REG_FP));
  mvwprintw(wRegisters, 3, 2+14, "IX: %04Xh", vm.reg16(REG_IX));
  mvwprintw(wRegisters, 4, 2+14, "IY: %04Xh", vm.reg16(REG_IY));
  
  mvwprintw(wRegisters, 6, 2, "PC: %04Xh", vm.pc());
  mvwprintw(wRegisters, 5, 2, "C%c Z%c S%c V%c",
            vm.flags() & FLAG_CARRY ? '1':'0',
            vm.flags() & FLAG_ZERO ? '1':'0',
            vm.flags() & FLAG_SIGN ? '1':'0',
            vm.flags() & FLAG_OVERFLOW ? '1':'0');
  
  mvwprintw(wRegisters, 6, 2+14, "%8lu", counter);
}

void UI::updateStack()
{
  wclear(wStack);
  box(wStack, 0, 0);
  mvwprintw(wStack, 0, 1, "[Stack]");
  
  constexpr u32 BYTES_PER_ROW = 8;
  u32 ROWS = height-WINDOW_REGS_HEIGHT-2;
  u32 CENTER = ROWS/2;
  u32 MIN_VALUE = 0x0000, MAX_VALUE = 0x10000 - BYTES_PER_ROW;
  
  s32 delta = CENTER*BYTES_PER_ROW;
  s32 baseOffset = (vm.reg16(REG_SP)/BYTES_PER_ROW) * BYTES_PER_ROW;
  
  for (int i = 0; i < ROWS; ++i)
  {
    s32 current = baseOffset - delta + i * BYTES_PER_ROW;

    if (current >= MIN_VALUE && current <= MAX_VALUE)
    {
      mvwprintw(wStack, 1+i, 2, "%04X:", current);

      for (int j = 0; j < BYTES_PER_ROW; ++j)
      {
        u32 address = current + j;
        mvwprintw(wStack, 1+i, 7+j*3 + 1, "%02X", vm.ramRead(address));
        
        if (address == vm.reg16(REG_SP))
          mvwprintw(wStack, 1+i, 7+j*3, ">", vm.ramRead(address));
        if (address == vm.reg16(REG_FP))
          mvwprintw(wStack, 1+i, 7+j*3+3, "<", vm.ramRead(address));
      }
    }
  }
}

void UI::updateCode()
{
  wclear(wCode);
  box(wCode, 0, 0);
  mvwprintw(wCode, 0, 1, "[Code]");
  
  u32 ROWS = height-2-LOWER_PANEL_HEIGHT;
  u32 CENTER = ROWS/2;
  u32 MIN_VALUE = 0x0000, MAX_VALUE = 0xFFFF;

  s32 pc = vm.pc();
  
  bool finished = false;
  int row = 0;
  while (!finished && row < ROWS)
  {
    if (pc >= MIN_VALUE && pc <= MAX_VALUE)
    {
      const u8* code = vm.ram()+pc;
      auto info = Opcodes::printInstruction(code);
      
      mvwprintw(wCode, 1+row, 2, "%04X: ", pc);
      
      for (int j = 0; j < info.length; ++j)
        mvwprintw(wCode, 1+row, 7+2+2*j, "%02X", code[j]);
      
      mvwprintw(wCode, 1+row, 7+2+2*4+2, "%s", info.value.c_str());
      
      if (pc == vm.pc())
        mvwprintw(wCode, 1+row, 1, ">");
      
      pc += info.length;
    }
    else
      finished = true;
    
    ++row;
  }
  
}

template<size_t HEIGHT>
class MyStdOut : public StdOut
{
public:
  std::string buffer[HEIGHT];

private:
  void shiftUp()
  {
    for (int i = 0; i < HEIGHT-1; ++i)
    {
      buffer[i] = buffer[i+1];
    }
    
    buffer[HEIGHT-1].clear();
  }
public:
  size_t index = 0;
  void out(u8 value) override
  {
    if (value == '\n')
    {
      if (index < HEIGHT-1)
        ++index;
      else
        shiftUp();
      
      return;
    }
    
    buffer[index] += value;
  }
};

static MyStdOut<LOWER_PANEL_HEIGHT-2> sout;

StdOut* UI::getStdOut() { return &sout; }

void UI::updateConsole()
{
  wclear(wConsole);
  box(wConsole, 0, 0);
  for (int i = 0; i < LOWER_PANEL_HEIGHT-2; ++i)
  {
    mvwprintw(wConsole, 1+i, 1, "%s", sout.buffer[i].c_str());
  }
  
  mvwprintw(wConsole, LOWER_PANEL_HEIGHT-1, 3, "(S) Step (T) Step x%lu", stepSize);
}

void UI::handleEvents()
{
  int c = getch();
  
  switch (c) {
    case 'q':
    case 'Q':
    {
      shouldQuit = true;
      break;
    }
    case 's':
    case 'S':
    {
      vm.executeInstruction();
      ++counter;
      draw();
      break;
    }
    case 't':
    case 'T':
    {
      for (int i = 0; i < stepSize; ++i)
        vm.executeInstruction();
      counter += stepSize;
      draw();
      break;
    }
    case '+':
    {
      stepSize <<= 1;
      draw();
      break;
    }
    case '-':
    {
      if (stepSize > 2)
      {
        stepSize >>= 1;
        draw();
      }
      break;
    }
    
  }
}

void UI::shutdown()
{
  endwin();
}