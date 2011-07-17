
#pragma once

class String;

class Error
{
public:

  Error() : err(0) {}

  Error(unsigned int err) : err(err) {}

  String getString() const;

  static const char* program;

private:
  unsigned int err;
};
