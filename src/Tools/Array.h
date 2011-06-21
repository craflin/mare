
#pragma once

template <typename T> class Array
{
public:
  Array() : data(0), size(0), capacity(0) {}

  ~Array()
  {
    if(data)
      delete[] data;
  }

  T* getFirst() {return data;}
  const T* getFirst() const {return data;}

  const T* getData() const {return data;}
  T* getData(unsigned int size) const
  {
    grow(size, 0);
    return data;
  }

  T& append(const T& elem = T())
  {
    unsigned int newSize = size + 1;
    grow(newSize, size);
    T& result = data[size];
    result = elem;
    size = newSize;
    return result;
  }

  void remove(unsigned int index)
  {
    if(index < size)
    {
      --size;
      for(T* pos = data + index, * end = data + size; pos < end; ++pos)
        *pos = pos[1];
    }
  }

  int find(const T& elem)
  {
    for(T* pos = data, * end = data + size; pos < end; ++pos)
      if(*pos == elem)
        return pos - data;
    return -1;
  }

  inline unsigned int getSize() const {return size;}
  void setSize(unsigned int size)
  {
    grow(size, size);
    this->size = size;
  }
  
  inline unsigned int getCapacity() const {return capacity;}
  inline void setCapacity(unsigned int capacity) {grow(capacity, size);}

  inline void clear() {size = 0;}
  inline bool isEmpty() const {return size == 0;}

private:
  T* data;
  unsigned int size;
  unsigned int capacity;

  void grow(unsigned int capacity, unsigned int size)
  {
    if(this->capacity < capacity)
    {
      T* oldData = data;
      this->capacity = capacity + 16 + (capacity >> 1);
      data = new T[this->capacity];
      for(T* pos = data, * end = data + size, * oldPos = oldData; pos < end;)
        *(pos++) = *(oldPos++);
      if(oldData)
        delete[] oldData;
      this->size = size;
    }
  }
};

