
#include <cctype>
#include <cstring>

#include "Word.h"

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

void Word::split(const String& text, List<Word>& words)
{
  struct Str
  {
    inline static const char* strspace(const char* str)
    {
      for(; *str; ++str)
        if(isspace(*str))
          return str;
      return str;
    }
  };

  const char* str = text.getData();
  for(;;)
  {
    while(isspace(*str))
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
      if(end > str)
        words.append(Word(text.substr(str - text.getData(), end - str), true));
      str = end;
      if(*str)
        ++str;
    }
    else
    {
      const char* end = Str::strspace(str);
      words.append(Word(text.substr(str - text.getData(), end - str), false));
      str = end;
    }
  }
}

void Word::append(const List<Word>& words, String& text)
{
  if(words.isEmpty())
    return;

  int totalLen = words.getSize() * 3;
  for(const List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
    totalLen += i->data.getLength();
  text.setCapacity(totalLen + 16);

  const List<Word>::Node* i = words.getFirst();
  const Word& word = i->data;
  if(word.quoted)
  {
    text.append('"');
    text.append(word);
    text.append('"');
  }
  else // TODO: escape spaces using blackslashes
    text.append(word);
  for(i = i->getNext(); i; i = i->getNext())
  {
    text.append(' ');
    const Word& word = i->data;
    if(word.quoted)
    {
      text.append('"');
      text.append(word);
      text.append('"');
    }
    else  // TODO: escape spaces using blackslashes
      text.append(word);
  }
}

String Word::join(const List<String>& words)
{
  if(words.isEmpty())
    return String();

  int totalLen = words.getSize() * 3;
  for(const List<String>::Node* i = words.getFirst(); i; i = i->getNext())
    totalLen += i->data.getLength();

  String result(totalLen);
  for(const List<String>::Node* i = words.getFirst(); i; i = i->getNext())
  {
    if(!result.isEmpty())
      result.append(' ');
    int len = result.getLength();
    char* dest = result.getData(len) + len; 
    for(const char* str = i->data.getData(); *str; ++str)
      if(isspace(*str))
      {
        result.setLength(len); // fall back
        result.append('"');
        result.append(i->data);
        result.append('"');
        goto next;
      }
      else
        *(dest++) = *str;
    result.setLength(len + i->data.getLength());
  next:;
  }
  return result;
}
