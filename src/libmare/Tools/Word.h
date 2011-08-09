
#pragma once

#include "String.h"
#include "List.h"

class Word
{
public:
  static void split(const String& text, List<String>& words);
  static void append(const List<String>& words, String& text);
};
