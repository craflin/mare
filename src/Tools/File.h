
#pragma once

#include "Error.h"

class File
{
public:
  enum Flags
  {
    readFlag = 0x0001,
    writeFlag = 0x0002,
  };

  File();
  ~File();

  bool open(const String& file, Flags flags = readFlag);
  int read(char* buffer, int len);

  inline const Error& getErrno() {return error;}

  static String getBasename(const String& file);

  static bool getWriteTime(const String& file, long long& writeTime);

private:
  void* fp;
  Error error;
};
