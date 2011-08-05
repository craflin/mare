

#pragma once

#include "List.h"
#include "String.h"

class Process
{
public:

  Process();
  ~Process();

  unsigned int start(const List<String>& command);

  unsigned int join();

  static unsigned int waitOne();

  static unsigned  int getProcessorCount();

private:
#ifdef _WIN32
  void* hProcess;
#else
  unsigned int pid;
  unsigned int exitCode;
#endif
};
