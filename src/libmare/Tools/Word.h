
#pragma once

#include "String.h"
#include "List.h"

class Word : public String
{
public:
  enum Flags
  {
    quotedFlag = (1 << 1),

    defaultFlag = (1 << 16), /**< used for built-in keys */
    commandLineFlag = (1 << 17), /**< used for command line keys */
  };

  unsigned int flags;

  Word(const String& word, unsigned int flags) : String(word), flags(flags) {}

  Word& operator=(const Word& other);
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
