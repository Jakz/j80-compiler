//
//  main.cpp
//  J80 Compiler
//
//  Created by Jack on 4/26/14.
//  Copyright (c) 2014 Jack. All rights reserved.
//



#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <thread>
#include <array>

#include "assembler.h"
#include "compiler.h"

#include "ast.h"
#include "compiler/rtl.h"

#include "screen.h"

using namespace std;

bool stringEndsWith(const string& text, const string& suffix)
{
  return suffix.size() <= text.size() && std::equal(suffix.rbegin(), suffix.rend(), text.rbegin());
}

string trimExtension(const string& text)
{
  size_t lastindex = text.find_last_of(".");
  size_t lastSlash = text.find_last_of("/");
  
  if (lastindex != string::npos && (lastSlash == string::npos || lastSlash < lastindex))
    return text.substr(0, lastindex);
  else
    return text;
}

class PrintfStdOut : public StdOut
{
  void out(u8 value) override
  {
    printf("%c", (char)value);
  }
};

static bool shouldStopVM = false;

void runWithArgs(const vector<string>& args, Assembler::J80Assembler& assembler, nanoc::Compiler& compiler)
{
  if (args.size() == 2)
  {
    if (stringEndsWith(args[1], ".j80"))
    {
      cout << "Assembling " << args[1] << ".." << endl;
      bool success = assembler.parse(args[1]);
      
      if (success)
      {
        Result result = assembler.assemble();
        
        if (result)
        {
          cout << "Program Assembled, output:" << endl;
          assembler.printProgram(cout);
          
          string output = trimExtension(args[1]) + ".bin";
          assembler.saveForLogisim(output);

          std::thread thread = std::thread([&assembler] {
            VM vm;
            vm.copyToRam(assembler.getCodeSegment().data, assembler.getCodeSegment().length);
            vm.copyToRam(assembler.getDataSegment().data, assembler.getDataSegment().length, assembler.getDataSegment().offset);
            vm.setStdOut(new PrintfStdOut());

            
            while (!shouldStopVM)
              vm.executeInstruction();
          });

          thread.detach();

          getchar();
          shouldStopVM = true;
        }
        else
          assembler.log(Log::ERROR, true, "Error: {}", result.message);
      }
    }
    else if (stringEndsWith(args[1], ".nc"))
    {
      compiler.parse(args[1]);
      const auto& root = compiler.getAST();

      rtl::RTLBuilder builder;
      builder.dispatch(root.get());
      builder.print();
      
    }
  }
}

int main(int argc, const char * argv[])
{
  VM vm;


  Assembler::J80Assembler assembler;
  nanoc::Compiler compiler;

  runWithArgs({ "j80", "tests/test.nc" }, assembler, compiler);
  //runWithArgs({ "j80", "tests/testsuite.j80" }, assembler, compiler);

  return 0;

  vm.copyToRam(assembler.getCodeSegment().data, assembler.getCodeSegment().length);
  vm.copyToRam(assembler.getDataSegment().data, assembler.getDataSegment().length, assembler.getDataSegment().offset);
  vm.setDataSegmentStart(assembler.getDataSegment().offset);
  
  ui::Screen screen;

  screen.init(80, 60);
  screen.setVM(&vm);
  
  screen.drawLayout();
  screen.refresh();

  screen.loop();

  screen.deinit();

  return 0;

  if (argc > 1)
  {
    vector<string> args;
    
    for (int i = 0; i < argc; ++i)
      args.push_back(argv[i]);
    
    runWithArgs(args, assembler, compiler);
    return 0;
  }
  else if (argc == 1)
  {
    runWithArgs({"j80", "tests/test.nc"}, assembler, compiler);
  }
    
  
  //nanoc::Compiler compiler;
  //compiler.parse("test.nc");
  
  /*rtl::RTLBuilder builder;
  const auto& ast = compiler.getAST();
  builder.dispatch(ast.get());
  builder.print();*/
  return 0;
}

