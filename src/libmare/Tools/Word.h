
#pragma once

#include "String.h"
#include "List.h"

class Word : public String
{
public:
  bool quoted;
  bool terminated;

  Word(const String& word, bool quoted) : String(word), quoted(quoted), terminated(false) {}

  Word& operator=(const String& other);

  bool operator==(const Word& other) const;
  bool operator!=(const Word& other) const;

  void appendTo(String& text) const;

  static void split(const String& text, List<Word>& words);
  static void append(const List<Word>& words, String& text);
  static void splitLines(const String& text, List<Word>& words);
};
