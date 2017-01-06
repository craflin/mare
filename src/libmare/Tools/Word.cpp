
#include <cctype>
#include <cstring>

#include "Word.h"

Word& Word::operator=(const Word& other)
{
  (String&)*this = other;
  flags = other.flags;
  return *this;
}

Word& Word::operator=(const String& other)
{
  (String&)*this = other;
  return *this;
}

bool Word::operator==(const Word& other) const
{
  return *(String*)this == other;
}

bool Word::operator!=(const Word& other) const
{
  return *(String*)this != other;
}

String Word::first(const String& text)
{
  List<Word> words;
  split(text, words);
  return words.isEmpty() ? String() : words.getFirst()->data;
}

void Word::split(const String& text, List<Word>& words)
{
  const char* str = text.getData();
  for(;;)
  {
    while(isspace(*(unsigned char*)str))
      ++str;
    if(!*str)
      break;
    if(*str == '"')
    {
      ++str;
      const char* end = str;
      for(; *end; ++end)
        if(*end == '\\' && end[1] == '"')
          ++end;
        else if(*end == '"')
          break;
      if(end > str) // TODO: read escaped spaces as ordinary spaces?
        words.append(Word(text.substr(str - text.getData(), end - str), Word::quotedFlag));
      str = end;
      if(*str)
        ++str; // skip closing '"'
    }
    else
    {
      const char* end = str;
      for(; *end; ++end)
        if(isspace(*(unsigned char*)end))
          break;
      // TODO: read escaped spaces as ordinary spaces
      words.append(Word(text.substr(str - text.getData(), end - str), 0));
      str = end;
    }
  }
}

void Word::append(const List<Word>& words, String& text)
{
  if(words.isEmpty())
    return;

  size_t totalLen = words.getSize() * 3;
  for(const List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
    totalLen += i->data.getLength();
  text.setCapacity(totalLen + 16);

  const List<Word>::Node* i = words.getFirst();
  //const List<Word>::Node* previousWord = i;
  i->data.appendTo(text);
  for(i = i->getNext(); i; i = i->getNext())
  {
    text.append(/*previousWord->data.terminated ? '\n' : */' ');
    i->data.appendTo(text);
    //previousWord = i;
  }
}

void Word::appendTo(String& text) const
{
  if(flags & quotedFlag)
  {
    text.append('"');
    text.append(*this);
    text.append('"');
  }
  else // TODO: escape spaces using blackslashes
    text.append(*this);
}

void Word::splitLines(const String& text, List<Word>& words)
{
  const char* str = text.getData();
  for(;;)
  {
    const char* end = str;
    for(; *end; ++end)
      if(*end == '\n' || *end == '\r')
        break;
    // TODO: read escaped spaces as ordinary spaces
    words.append(Word(text.substr(str - text.getData(), end - str), 0));
    str = end;
    if(*str)
    {
      if(*str == '\r' && str[1] == '\n')
        str += 2;
      else
        ++str;
    }
    else
      break;
  }
}
