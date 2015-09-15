/**
* @file String.h
* Delcaration of a lightweight not multithread safe reference counting lazy copying string
* @author Colin Graf
*/

#pragma once

#include <cstddef> // for ptrdiff_t and size_t on Linux

class String
{
public:

  String() : data(&emptyData) {++data->refs;}

  String(const String& other) : data(other.data) {++data->refs;}

  template <int N> String(const char (&str)[N]) {init(N - 1, str, N - 1);}

  String(const char* str, ptrdiff_t length);

  String(size_t capacity) {init(capacity, 0, 0);}

  ~String();

  String& operator=(const String& other);
  inline String& operator+=(const String& other) {return append(other);}
  inline String operator+(const String& other) const {return String(*this).append(other);}

  bool operator==(const String& other) const;
  bool operator!=(const String& other) const;

  inline const char* getData() const {return data->str;}

  char* getData(size_t capacity);

  void setCapacity(size_t capacity);

  void setLength(size_t length);
  inline size_t getLength() const {return data->length;}

  String& format(size_t capacity, const char* format, ...);

  String& prepend(const String& str);

  String& append(char c);
  String& append(const String& str);
  String& append(const char* str, size_t length);

  void clear();
  bool isEmpty() const {return data->length == 0;}

  String substr(ptrdiff_t start, ptrdiff_t length = -1) const;

  bool patmatch(const String& pattern) const;
  bool patsubst(const String& pattern, const String& replace);

  int subst(const String& from, const String& to);

  /**
   * Find str in this String. "" is always contained.
   *
   * @param str The string to find.
   * @param pos The position of str in this String if contained, otherwise undefined.
   * @return Returns whether str was found.
   */
  bool find(const String& str, size_t& pos) const;
  bool contains(const String& str) const;
  bool find(char ch, size_t& pos) const;

  String& lowercase();
  String& uppercase();

private:
  class Data
  {
  public:
    const char* str;
    size_t length; // size TODO: rename
    size_t capacity;
    unsigned int refs;
    Data* next;

    Data() {}

    template <size_t N> Data(const char (&str)[N]) : str(str), length(N - 1), capacity(0), refs(1) {}
  };

  Data* data;

  static Data emptyData;
  static Data* firstFreeData;

  void init(size_t capacity, const char* str, size_t length);
  void free();
  void grow(size_t capacity, size_t length);
};
