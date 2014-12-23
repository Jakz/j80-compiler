#include "utils.h"

#include <stdio.h>

int Utils::fd = 0;
fpos_t Utils::pos = 0;

std::string Utils::execute(std::string command)
{
  FILE* pipe = popen(command.c_str(), "r");
  if (!pipe) return "ERROR";
  char buffer[128];
  std::string result = "";
  while(!feof(pipe)) {
    if(fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
}