#pragma once

#include "vm.h"
#include "BearLibTerminal.h"

#include <vector>
#include <regex>
#include <bitset>

namespace ui
{
  enum class LineType { Thick, Light };
  enum class LineMask
  {
    None = 0b0000, North = 0b0001, East = 0b0010, South = 0b0100, West = 0b1000,
    Hor = West | East, Ver = North | South,
    NE = North | East, NW = North | West, SE = South | East, SW = South | West
  };

  LineMask operator|(LineMask lhs, LineMask rhs) { return LineMask(std::underlying_type_t<LineMask>(lhs) | std::underlying_type_t<LineMask>(rhs)); }

  struct LineSlot
  {
    LineMask thick, light;

    LineSlot() : thick(LineMask::None), light(LineMask::None) { }

    bool any() const { return thick != LineMask::None || light != LineMask::None; }

    void put(LineType type, LineMask mask)
    {
      if (type == LineType::Light)
        light = light | mask;
      else
        thick = thick | mask;
    }
  };

  struct LineGridField
  {
  private:
    size_t w, h;
    std::vector<LineSlot> data;

  public:
    LineGridField(size_t w, size_t h) : w(w), h(h)
    {
      data.resize(w * h, LineSlot());
    }

    LineSlot& get(s32 x, s32 y) { return data[w * y + x]; }
    const LineSlot& get(s32 x, s32 y) const { return data[w * y + x]; }

    size_t height() const { return h; }
    size_t width() const { return w; }
  };

  struct LineEncoding
  {
  private:
    std::unordered_map<size_t, char32_t> mapping;

    size_t toKey(LineMask light, LineMask thick) const { return size_t(light) | size_t(thick) << 4; }

  public:
    LineEncoding()
    {
      init();
    }

    void map(LineMask light, LineMask thick, char32_t value)
    {
      mapping[toKey(light, thick)] = value;
    }

    void init()
    {
      map(LineMask::None, LineMask::Hor, U'━');
      map(LineMask::None, LineMask::Ver, U'┃');
      map(LineMask::None, LineMask::NE, U'┏');
      map(LineMask::None, LineMask::NW, U'┓');
      map(LineMask::None, LineMask::SE, U'┗');
      map(LineMask::None, LineMask::SW, U'┛');

      map(LineMask::Hor, LineMask::None, U'─');
      map(LineMask::Ver, LineMask::None, U'│');
      map(LineMask::NE, LineMask::None, U'┌');
      map(LineMask::NW, LineMask::None, U'┐');
      map(LineMask::SE, LineMask::None, U'└');
      map(LineMask::SW, LineMask::None, U'┘');

      map(LineMask::Ver, LineMask::Hor, U'┿');
      map(LineMask::Hor, LineMask::Ver, U'╂');

    }

    char32_t get(const LineSlot& slot) const
    {
      auto key = toKey(slot.light, slot.thick);
      auto it = mapping.find(key);

      return it != mapping.end() ? it->second : U'.';
    }

    char32_t operator[](const LineSlot& slot) const { return get(slot); }
  };

  struct Point
  {
    s32 x, y;

    Point() : x(0), y(0) { }
    Point(s32 x, s32 y) : x(x), y(y) { }
  };

  struct Size
  {
    s32 w, h;

    Size() : w(0), h(0) { }
    Size(s32 w, s32 h) : w(w), h(h) { }
  };

  struct Box
  {
    Point origin;
    Size size;

    Box() : Box(0, 0, 0, 0) { }
    Box(s32 x, s32 y, s32 w, s32 h) : origin(x, y), size(w, h) { }

    auto x() const { return origin.x; }
    auto y() const { return origin.y; }
    auto w() const { return size.w; }
    auto h() const { return size.h; }

    auto trx() const { return origin.x + size.w - 1; }
  };

  class Screen
  {
  private:
    VM* vm;
    ui::LineEncoding mapping;
    s32 w, h;

    std::vector<std::unique_ptr<Assembler::Instruction>> buffer;
    Regs regs;

    Box REGISTERS_BOX;
    Box INSTRUCTIONS_BOX;

    enum Colors
    {
      CurrentInstruction = 0xFFFFFA28,
      RegisterChanged = 0xFFFFFA28,

      Normal = 0xffcbff87,
      Boxes = 0xff8eff00,
    };


    


  public:
    void setVM(VM* vm) { this->vm = vm; }

    void init(s32 width, s32 height);
    void deinit();

    void flip() { terminal_refresh(); }

    void drawBox(LineType type, int layer, Box b) const;

    void drawRegs();
    void drawInstructions();
    void drawJumps();

    void drawLayout();

    void refresh();

    void loop();
  };

  void Screen::init(s32 width, s32 height)
  {
    w = width;
    h = height;

    REGISTERS_BOX = Box(0, 0, 11, 15);
    INSTRUCTIONS_BOX = Box(REGISTERS_BOX.trx() + 1, 0, w - REGISTERS_BOX.w(), h);
    
    terminal_open();

    terminal_set(fmt::format("window: size={}x{}, title=J80", width, height).c_str());
    terminal_set("font: UbuntuMono.ttf, size=10");
  }

  void Screen::deinit()
  {
    terminal_close();
  }

  void Screen::drawBox(LineType type, int layer, Box b) const
  {
    assert(b.size.w >= 2 && b.size.h >= 2);

    b.size.w -= 1;
    b.size.h -= 1;

    ui::LineGridField field(100, 100);

    field.get(b.x(), b.y()).put(type, ui::LineMask::NE);
    field.get(b.x() + b.w(), b.y()).put(type, ui::LineMask::NW);
    field.get(b.x(), b.y() + b.h()).put(type, ui::LineMask::SE);
    field.get(b.x() + b.w(), b.y() + b.h()).put(type, ui::LineMask::SW);

    for (int i = b.x() + 1; i < b.x() + b.w(); ++i)
    {
      field.get(i, b.y()).put(type, ui::LineMask::Hor);
      field.get(i, b.y() + b.h()).put(type, ui::LineMask::Hor);
    }

    for (int i = b.y() + 1; i < b.y() + b.h(); ++i)
    {
      field.get(b.x(), i).put(type, ui::LineMask::Ver);
      field.get(b.x() + b.w(), i).put(type, ui::LineMask::Ver);
    }

    terminal_layer(layer);

    for (size_t y = 0; y < field.height(); ++y)
      for (size_t x = 0; x < field.width(); ++x)
        if (field.get(x, y).any())
          terminal_put(x, y, mapping[field.get(x, y)]);
  }

  void Screen::drawLayout()
  {
    terminal_color(Colors::Boxes);
    drawBox(LineType::Light, 0, REGISTERS_BOX);
    drawBox(LineType::Light, 0, INSTRUCTIONS_BOX);
  }

  void Screen::drawInstructions()
  {
    buffer.clear();
    
    s32 current = vm->pc();

    terminal_clear_area(INSTRUCTIONS_BOX.x() + 1, INSTRUCTIONS_BOX.y() + 1, INSTRUCTIONS_BOX.w() - 2, INSTRUCTIONS_BOX.h() - 2);

    for (s32 i = 0; i < INSTRUCTIONS_BOX.h() - 2; ++i)
    {
      if (current >= vm->getDataSegmentStart())
        break;

      auto* instruction = Assembler::Instruction::disassemble(vm->ram() + current);
      instruction->setAddress(current);

      buffer.push_back(std::unique_ptr<Assembler::Instruction>(instruction));

      std::stringstream ss;
      for (s32 s = 0; s < 4; ++s)
      {
        if (s >= instruction->getLength())
          ss << "  ";
        else
          ss << fmt::format("{:02x}", *(vm->ram() + current + s));
      }

      terminal_color(current == vm->pc() ? Colors::CurrentInstruction : Colors::Normal);
      auto string = fmt::format("{:04x}h: {} {}", current, ss.str(), instruction->mnemonic());

      string = std::regex_replace(string, std::regex("\\["), "[[");
      string = std::regex_replace(string, std::regex("\\]"), "]]");

      terminal_print(INSTRUCTIONS_BOX.x() + 2, INSTRUCTIONS_BOX.y() + 1 + i, string.c_str());
      current += instruction->getLength();
    }
  }

  void Screen::drawJumps()
  {
    static constexpr s32 MAX_WIDTH = 20;
    
    std::vector<std::bitset<MAX_WIDTH>> matrix;
    matrix.resize(buffer.size());
    
    for (s32 i = 0; i < buffer.size(); ++i)
    {
      const auto* jump = dynamic_cast<const Assembler::InstructionJMP_NNNN*>(buffer[i].get());
      
      
      if (jump)
      {
        for (s32 j = 0; j < buffer.size(); ++j)
        {
          auto start = i;
          
          if (buffer[j]->getAddressInROM() == jump->getAddress().address)
          {
            auto min = std::min(i, j);
            auto max = std::max(i, j);
            auto end = j;

            for (s32 u = 1; u < MAX_WIDTH; ++u)
            {
              bool free = true;

              for (int o = min; o <= max; ++o)
                if (matrix[o][u] && ((o != min && o != max) || matrix[o][u-1]))
                {
                  free = false;
                  break;
                }

              if (free)
              {
                for (int o = min; o <= max; ++o)
                {
                  terminal_put(INSTRUCTIONS_BOX.x() + 40 + u, INSTRUCTIONS_BOX.y() + 1 + o, o == min ? U'┐' : (o == max ? U'┘' : U'│'));
                  matrix[o][u] = true;
                }

                terminal_put(INSTRUCTIONS_BOX.x() + 40 + u - 1, INSTRUCTIONS_BOX.y() + 1 + end, '<');

                break;
              }
            }
          }
        }


      }
    }
  }

  void Screen::drawRegs()
  {
    std::array<Reg, 8> regs = {
      Reg::BA, Reg::CD, Reg::EF, Reg::XY,
      Reg::IX, Reg::IY, Reg::SP, Reg::FP
    };

    std::array<Flag, 4> flags = { Flag::FLAG_CARRY, Flag::FLAG_ZERO, Flag::FLAG_SIGN, Flag::FLAG_OVERFLOW };

    terminal_print(REGISTERS_BOX.x() + 1, REGISTERS_BOX.y() + 1, fmt::format("PC: {:04x}h", vm->pc()).c_str());

    for (s32 i = 0; i < regs.size(); ++i)
    {
      terminal_color(vm->allRegs().reg16(regs[i]) != this->regs.reg16(regs[i]) ? Colors::CurrentInstruction : Colors::Normal);
      terminal_print(REGISTERS_BOX.x() + 1, REGISTERS_BOX.y() + 3 + i, fmt::format("{}: {:04x}h", Opcodes::reg16(regs[i]), vm->reg16(regs[i])).c_str());
    }

    const auto flagsY = REGISTERS_BOX.y() + regs.size() + 2 + 2;
    terminal_print(REGISTERS_BOX.x() + 1, flagsY, "CZSO");
    for (s32 i = 0; i < flags.size(); ++i)
    {
      terminal_color(vm->allRegs().flag(flags[i]) != this->regs.flag(flags[i]) ? Colors::CurrentInstruction : Colors::Normal);
      terminal_put(REGISTERS_BOX.x() + 1 + i, flagsY + 1, vm->allRegs().flag(flags[i]) ? '1' : '0');
    }
  }

  void Screen::refresh()
  {
    terminal_color(Colors::Normal);
    drawRegs();
    drawInstructions();
    drawJumps();
    flip();
  }

  void Screen::loop()
  {
    // Wait until user close the window
    bool shouldQuit = false;
    while (!shouldQuit)
    {
      int evt = terminal_read();

      if (evt == TK_CLOSE || evt == (TK_ESCAPE | TK_KEY_RELEASED))
        shouldQuit = true;
      else if (evt == TK_A)
      {

      }
      else if (evt == TK_S)
      {
        regs = vm->allRegs();

        /*auto* instruction = Assembler::Instruction::disassemble(vm->ram() + vm->pc());
        u8 temp[4];
        instruction->assemble(temp);

        for (int k = 0; k < instruction->getLength(); ++k)
          assert(temp[k] == *(vm->ram() + vm->pc() + k));*/

        vm->executeInstruction();
        refresh();
      }
      else if (evt == TK_R)
      {
        regs = Regs();
        vm->reset();
        refresh();
        //TODO: should reset potentially modified ram
      }

    }
  }
}