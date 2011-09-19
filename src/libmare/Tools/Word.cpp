
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
      Word* word = 0;
      if(end > str) // TODO: read escaped spaces as ordinary spaces?
        word = &words.append(Word(text.substr(str - text.getData(), end - str), true));
      str = end;
      if(*str)
        ++str; // skip closing '"'
      if(word && (*str == '\n' || * str == '\0'))
        word->terminated = true;
    }
    else
    {
      const char* end = str;
      for(; *end; ++end)
        if(isspace(*end))
          break;
      // TODO: read escaped spaces as ordinary spaces
      Word& word = words.append(Word(text.substr(str - text.getData(), end - str), false));
      str = end;
      if(*str == '\n' || * str == '\0')
        word.terminated = true;
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
  i->data.appendTo(text);
  for(i = i->getNext(); i; i = i->getNext())
  {
    if(!text.isEmpty() && text.getData()[text.getLength() - 1] != '\n')
      text.append(' ');
    i->data.appendTo(text);
  }
}

void Word::appendTo(String& text) const
{
  if(quoted)
  {
    text.append('"');
    text.append(*this);
    text.append('"');
  }
  else // TODO: escape spaces using blackslashes
    text.append(*this);
  if(terminated)
    text.append('\n');
}
