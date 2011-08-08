
#include <cctype>
#include <cstring>

#include "Words.h"

inline static const char* strspace(const char* str)
{
  for(; *str; ++str)
    if(isspace(*str))
      return str;
  return str;
}

void Words::split(const String& text, List<String>& words)
{
  const char* str = text.getData();
  while(*str)
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
        words.append(text.substr(str - text.getData(), end - str));
      str = end;
      if(*str)
        ++str;
    }
    else
    {
      const char* end = strspace(str);
      words.append(text.substr(str - text.getData(), end - str));
      str = end;
    }
  }
}

void Words::append(const List<String>& words, String& text)
{
  for(const List<String>::Node* i = words.getFirst(); i; i = i->getNext())
  {
    if(i != words.getFirst())
      text.append(' ');
    const String& word = i->data;
    if(*strspace(word.getData()))
    {
      unsigned int buflen = text.getLength() + 3 + word.getLength() * 2;
      text.setCapacity(buflen);
      unsigned int textlen = text.getLength();
      char* str = text.getData(buflen) + textlen;
      *(str++) = '"';
      memcpy(str, word.getData(), word.getLength());
      str += word.getLength();
      *(str++) = '"';
      text.setLength(str - text.getData());
    }
    else
      text.append(i->data);
  }
}
