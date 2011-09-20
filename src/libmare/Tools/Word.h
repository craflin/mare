
#pragma once

#include "String.h"
#include "List.h"

class Word : public String
{
public:
  bool quoted;

  Word(const String& word, bool quoted) : String(word), quoted(quoted) {}

  Word& operator=(const String& other);

  bool operator==(const Word& other) const;
  bool operator!=(const Word& other) const;

  void appendTo(String& text) const;

  /**
  * Returns the first word within string
  * @param text The string
  * @return The first word
  */
  static String first(const String& text);

  static void split(const String& text, List<Word>& words);
  static void append(const List<Word>& words, String& text);
  static void splitLines(const String& text, List<Word>& words);
};
