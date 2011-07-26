
#include <cstring>
#include <cstdarg>
#include <cstdio>

#include "Assert.h"
#include "String.h"

String::Data String::emptyData("");
String::Data* String::firstFreeData = 0;

String::String(const char* str, int length)
{
  if(length < 0)
    length = strlen(str);
  init(length, str, length);
}

String::~String()
{
  free();
}

void String::init(unsigned int size, const char* str, unsigned int length)
{
  ASSERT(size >= length);
  if(firstFreeData)
  {
    data = firstFreeData;
    firstFreeData = firstFreeData->next;
    data->refs = 1;

    if(data->size < size)
    {
      delete[] (char*)data->str;
      data->size = size + 16 + (size >> 1); // badass growing strategy
      data->str = new char[data->size + 1];
    }
  }
  else
  {
    data = new Data;
    data->refs = 1;
    data->size = size + 16 + (size >> 1); // badass growing strategy
    data->str = new char[data->size + 1];
  }
  if(length)
    memcpy((char*)data->str, str, length);
  ((char*)data->str)[length] = '\0';
  data->length = length;
}

void String::free()
{
  --data->refs;
  if(data->refs == 0)
  {
    data->next = firstFreeData;
    firstFreeData = data;
  }
}

void String::grow(unsigned int size, unsigned int length)
{
  ASSERT(size >= length);
  ASSERT(length <= data->size);
  if(data->refs > 1)
  {
    Data* otherData = data;
    --otherData->refs;

    init(size, otherData->str, length);
  }
  else if(data->size < size)
  {
    char* otherStr = (char*)data->str;
    data->size = size + 16 + (size >> 1); // badass growing strategy
    data->str = new char[data->size + 1];
    if(length)
      memcpy((char*)data->str, otherStr, length);
    delete[] otherStr;

    data->length = length;
    ((char*)data->str)[length] = '\0';
  }
}


String& String::operator=(const String& other)
{
  free();
  data = other.data;
  ++data->refs;
  return *this;
}

bool String::operator==(const String& other) const
{
  return data->length == other.data->length && memcmp(data->str, other.data->str, data->length) == 0;
}

bool String::operator!=(const String& other) const
{
  return data->length != other.data->length || memcmp(data->str, other.data->str, data->length) != 0;
}

char* String::getData(unsigned int size)
{
  grow(size, 0);
  return (char*)data->str;
}

void String::setCapacity(unsigned int size)
{
  grow(size < data->length ? data->length : size, data->length); // enforce detach
}

String& String::append(char c)
{
  unsigned int newLength = data->length + 1;
  grow(newLength, data->length);
  ((char*)data->str)[data->length] = c;
  ((char*)data->str)[newLength] = '\0';
  data->length = newLength;
  return *this;
}

String& String::append(const String& str)
{
  unsigned int newLength = data->length + str.data->length;
  grow(newLength, data->length);
  memcpy(((char*)data->str) + data->length, str.data->str, str.data->length);
  ((char*)data->str)[newLength] = '\0';
  data->length = newLength;
  return *this;
}

String& String::append(const char* str, unsigned int length)
{
  unsigned int newLength = data->length + length;
  grow(newLength, data->length);
  memcpy(((char*)data->str) + data->length, str, length);
  ((char*)data->str)[newLength] = '\0';
  data->length = newLength;
  return *this;
}

void String::clear()
{
  if(data->refs == 1)
  {
    *(char*)data->str = '\0';
    data->length = 0;
  }
  else
  {
    free();
    data = &emptyData;
    ++data->refs;
  }
}

void String::setLength(unsigned int length)
{
  grow(length, length); // detach
  ((char*)data->str)[length] = '\0';
  data->length = length;
}

String& String::format(unsigned int size, const char* format, ...)
{
  int length;
  va_list ap;
  va_start(ap, format);
#ifdef _MSC_VER
  length = vsprintf_s(getData(size), size, format, ap);
#else
  length = ::vsnprintf(getData(size), size, format, ap);
  if(length < 0)
    length = size;
#endif
  va_end(ap);
  ((char*)data->str)[length] = '\0';
  data->length = length;
  return *this;
}

String String::substr(int start, int length) const
{
  if(start < 0)
  {
    start = (int)data->length + start;
    if(start < 0)
      start = 0;
  }
  else if((unsigned int)start > data->length)
    start = data->length;

  int end;
  if(length >= 0)
  {
    end = start + length;
    if((unsigned int)end > data->length)
      end = data->length;
  }
  else
    end = data->length;

  length = end - start;
  String result(length);
  memcpy((char*)result.data->str, data->str + start, length);
  ((char*)result.data->str)[length] = '\0';
  result.data->length = length;
  return result;
}

int String::subst(const String& from, const String& to)
{
  String result(data->length + to.data->length * 2);
  const char* str = data->str;
  const char* f = from.data->str;
  unsigned int flen = from.data->length;
  const char* start = str;
  int i = 0;
  while(*str)
    if(*str == *f && strncmp(str, f, flen) == 0)
    {
      if(str > start)
        result.append(start, str - start);
      result.append(to);
      str += flen;
      start = str;
      ++i;
    }
    else
      ++str;
  if(str > start)
    result.append(start, str - start);
  *this = result;
  return i;
}


static bool szWildMatch8(const char* pat, const char* str);
static bool szWildMatch1(const char* pat, const char* str, const char*& matchstart, const char*& matchend);

bool String::patmatch(const String& pattern) const
{
  return szWildMatch8(pattern.data->str, data->str);
}

bool String::patsubst(const String& pattern, const String& replace)
{
  const char* matchstart;
  const char* matchend;
  if(!szWildMatch1(pattern.data->str, data->str, matchstart, matchend))
    return false;

  unsigned int matchlen = matchend - matchstart;
  String result(matchlen + replace.data->length);
  char* dest = (char*)result.data->str;
  for(const char* src = replace.data->str; *src; ++src)
    if(*src == '\\' && (src[1] == '%' || (src[1] == '\\' && src[2] == '%')))
    {
      ++src;
      *(dest++) = *src;
    }
    else if(*src == '%')
    {
      memcpy(dest, matchstart, matchlen);
      dest += matchlen;
      ++src;
      unsigned int left = replace.data->length - (src - replace.data->str);
      memcpy(dest, src, left);
      dest += left;
      break;
    }
    else
      *(dest++) = *src;
  *dest = '\0';
  result.data->length = dest - result.data->str;
  *this = result;
  return true;
}

/*
slightly modified wildcard matching functions based on Alessandro Cantatore's algorithms
http://xoomer.virgilio.it/acantato/dev/wildcard/wildmatch.html
*/

static bool szWildMatch8(const char* pat, const char* str) {
    const char* s, * p;
    bool star = false;

loopStart:
    for (s = str, p = pat; *s; ++s, ++p) {
      switch (*p) {
          case '%':
            star = true;
            str = s, pat = p;
            if (!*++pat) return true;
            goto loopStart;
          default:
            if (*s != *p) goto starCheck;
            break;
      }
    }
    if (*p == '%') ++p;
    return (!*p);
   
starCheck:
    if (!star) return false;
    str++;
    goto loopStart;
}

static bool szWildMatch1(const char* pat, const char* str, const char*& matchstart, const char*& matchend)
{
  while(*str)
  {
    switch(*pat)
    {
    case '%':
      matchstart = str;
      do { ++pat; } while(*pat == '%');
      if(!*pat)
      {
        matchend = matchstart + strlen(matchstart);
        return true;
      }
      while(*str)
        if(szWildMatch8(pat, str++))
        {
          matchend = str - 1;
          return true;
        }
      return false;
    default:
      if(*str != *pat)
        return false;
      break;
    }
    ++pat, ++str;
  }
  while(*pat == '%')
    ++pat;
  if(!*pat)
  {
    matchstart = matchend = str;
    return true;
  }
  return false;
}

