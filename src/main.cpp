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

#include "assembler.h"
#include "compiler.h"

#include "ast.h"

#include "vm/ui.h"

using namespace std;
/*
int main(int argc, const char * argv[])
{
  ASTList* list = new ASTList();
  
  ASTList* list2 = new ASTList();
  list2->add(unique_ptr<ASTNode>(new ASTNumber(50)));
  
  list->add(unique_ptr<ASTNode>(new ASTNumber(10)));
  list->add(unique_ptr<ASTNode>(new ASTUnaryOperator(Unary::NOT, unique_ptr<ASTNode>(new ASTNumber(20)))));
  list->add(unique_ptr<ASTNode>(list2));
  list->recursivePrint(0);
  

  
  
  
  return 0;
}*/

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

void runWithArgs(const vector<string>& args)
{
  if (args.size() == 2)
  {
    if (stringEndsWith(args[1], ".j80"))
    {
      cout << "Assembling " << args[1] << ".." << endl;
      Assembler::J80Assembler assembler;
      bool success = assembler.parse(args[1]);
      
      if (success)
      {
        assembler.assemble();
        
        cout << "Program Assembled, output:" << endl;
        assembler.printProgram();
        
        string output = trimExtension(args[1]) + ".bin";
        assembler.saveForLogisim(output);
      }
    }
    else if (stringEndsWith(args[1], ".nc"))
    {
      nanoc::Compiler compiler;
      compiler.parse(args[1]);
    }
  }
}


int main(int argc, const char * argv[])
{
  if (argc > 1)
  {
    vector<string> args;
    
    for (int i = 0; i < argc; ++i)
      args.push_back(argv[i]);
    
    runWithArgs(args);
    return 0;
  }
  else if (argc == 1)
  {
    runWithArgs({"j80", "test.nc"});
  }
    
  
  nanoc::Compiler compiler;
  compiler.parse("test.nc");
  
  return 0;
  
  Assembler::J80Assembler assembler;
  assembler.parse("test.j80");
  assembler.assemble();
  
  
  return 0;
  

}

