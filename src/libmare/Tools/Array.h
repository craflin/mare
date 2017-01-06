
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
  T* getData(size_t size) const
  {
    grow(size, 0);
    return data;
  }

  T& append(const T& elem = T())
  {
    size_t newSize = size + 1;
    grow(newSize, size);
    T& result = data[size];
    result = elem;
    size = newSize;
    return result;
  }

  void remove(size_t index)
  {
    if(index < size)
    {
      --size;
      for(T* pos = data + index, * end = data + size; pos < end; ++pos)
        *pos = pos[1];
    }
  }

  ptrdiff_t find(const T& elem)
  {
    for(T* pos = data, * end = data + size; pos < end; ++pos)
      if(*pos == elem)
        return pos - data;
    return -1;
  }

  inline size_t getSize() const {return size;}
  void setSize(size_t size)
  {
    grow(size, size);
    this->size = size;
  }
  
  inline size_t getCapacity() const {return capacity;}
  inline void setCapacity(size_t capacity) {grow(capacity, size);}

  inline void clear() {size = 0;}
  inline bool isEmpty() const {return size == 0;}

private:
  T* data;
  size_t size;
  size_t capacity;

  void grow(size_t capacity, size_t size)
  {
    if(this->capacity < capacity)
    {
      T* oldData = data;
      this->capacity = capacity + 16 + (capacity >> 1);
      data = new T[this->capacity];
      for(T* pos = data, * end = data + this->size, * oldPos = oldData; pos < end;)
        *(pos++) = *(oldPos++);
      if(oldData)
        delete[] oldData;
      this->size = size;
    }
  }
};

