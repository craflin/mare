

#pragma once

#include "List.h"
#include "String.h"
#include "Error.h"

class Process
{
public:

  Process();
  ~Process();

  unsigned int start(const List<String>& command);

  unsigned int wait();

  inline const Error& getErrno() {return error;}

  static unsigned int waitOne();

private:
#ifdef _WIN32
  void* hProcess;
#endif
  Error error;
};
