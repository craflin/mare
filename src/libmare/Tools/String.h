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

  String(unsigned int capacity) {init(capacity, 0, 0);}

  ~String();

  String& operator=(const String& other);
  inline String& operator+=(const String& other) {return append(other);}
  inline String operator+(const String& other) const {return String(*this).append(other);}

  bool operator==(const String& other) const;
  bool operator!=(const String& other) const;

  inline const char* getData() const {return data->str;}

  char* getData(unsigned int capacity);

  void setCapacity(unsigned int capacity);

  void setLength(unsigned int length);
  inline unsigned int getLength() const {return data->length;}

  String& format(unsigned int capacity, const char* format, ...);

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

  /**
   * Find str in this String. "" is always contained.
   *
   * @param str The string to find.
   * @param pos The position of str in this String if contained, otherwise undefined.
   * @return Returns whether str was found.
   */
  bool find(const String& str, unsigned int& pos) const;
  bool contains(const String& str) const;
  bool find(char ch, unsigned int& pos) const;

  String& lowercase();
  String& uppercase();

private:
  class Data
  {
  public:
    const char* str;
    unsigned int length; // size TODO: rename
    unsigned int capacity;
    unsigned int refs;
    Data* next;

    Data() {}

    template <int N> Data(const char (&str)[N]) : str(str), length(N - 1), capacity(0), refs(1) {}
  };

  Data* data;

  static Data emptyData;
  static Data* firstFreeData;

  void init(unsigned int capacity, const char* str, unsigned int length);
  void free();
  void grow(unsigned int capacity, unsigned int length);
};
