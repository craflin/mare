/**
* @file String.h
* Delcaration of a lightweight not multithread safe reference counting lazy copying string
* @author Colin Graf
*/

#pragma once

class String
{
public:

  String() : data(&emptyData) {++data->refs;}

  String(const String& other) : data(other.data) {++data->refs;}

  template <int N> String(const char (&str)[N]) {init(N - 1, str, N - 1);}

  String(const char* str, int length);

  String(unsigned int size) {init(size, 0, 0);}
  
  ~String();

  String& operator=(const String& other);
  inline String& operator+=(const String& other) {return append(other);}
  inline String operator+(const String& other) const {return String(*this).append(other);}

  bool operator==(const String& other) const;
  bool operator!=(const String& other) const;

  inline const char* getData() const {return data->str;}
  
  char* getData(unsigned int size);

  void setCapacity(unsigned int capacity);

  void setLength(unsigned int length);
  inline unsigned int getLength() const {return data->length;}

  String& format(unsigned int size, const char* format, ...);

  String& prepend(const String& str);

  String& append(char c);
  String& append(const String& str);
  String& append(const char* str, unsigned int length);

  void clear();
  bool isEmpty() const {return data->length == 0;}

  String substr(int start, int length = -1) const;

  bool patmatch(const String& pattern) const;
  bool patsubst(const String& pattern, const String& replace);

  int subst(const String& from, const String& to);

  String& lowercase();
  String& uppercase();

private:
  class Data
  {
  public:
    const char* str;
    unsigned int length; // size TODO: rename
    unsigned int size; // capacity TODO: rename
    unsigned int refs;
    Data* next;

    Data() {}

    template <int N> Data(const char (&str)[N]) : str(str), length(N - 1), size(0), refs(1) {}
  };

  Data* data;

  static Data emptyData;
  static Data* firstFreeData;

  void init(unsigned int size, const char* str, unsigned int length);
  void free();
  void grow(unsigned int size, unsigned int length);
};
