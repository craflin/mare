
#pragma once

template <typename K, typename T> class Map
{
public:
  class Node
  {
  public:
    K key;
    T data;

    Node(const K& key, const T& data) : key(key), data(data) {}

    inline Node* getNext() {return next;}
    inline const Node* getNext() const {return next;}
    inline Node* getPrevious() {return previous;}
    inline const Node* getPrevious() const {return previous;}

  private:
    Node* next;
    Node* previous;

    unsigned int keyNum;
    Node** previousNextInTable;
    Node* nextInTable;

    friend class Map;
  };

  Map() : first(0), last(0), size(0), firstFree(0), table(0) {}

  ~Map()
  {
    if(first)
    {
      last->next = firstFree;
      firstFree = first;
    }
    for(Node* node = firstFree, * next; node; node = next)
    {
      next = node->next;
      delete node;
    }
    if(table)
      delete[] table;
  }

  Map& operator=(const Map& other)
  {
    clear();
    for(Node* i = other.first; i; i = i->next)
      append(i->key, i->data);
    return *this;
  }

  T& append(const K& key, const T& data = T())
  {
    // create node and add it to the double linked list
    Node* node;
    if(firstFree)
    {
      node = firstFree;
      firstFree = firstFree->next;
      node->key = key;
      node->data = data;
    }
    else
      node = new Node(key, data);
    node->next = 0;
    if((node->previous = last))
      last->next = node;
    else
    {
      first = node;
      if(!table)
      {
        table = new Node*[tableSize];
        for(Node** i = table, ** end = table + tableSize; i < end; ++i)
          *i = 0;
      }
    }
    last = node;
    ++size;

    // add to hash map
    unsigned int keyNum;
    node->keyNum = keyNum = (unsigned int)key;
    Node** cell;
    node->previousNextInTable = cell = &table[keyNum  % tableSize];
    if((node->nextInTable = *cell))
      node->nextInTable->previousNextInTable = &node->nextInTable;
    *cell = node;

    return node->data;
  }

  void remove(Node* node)
  {
    // remove from double linked list
    if(node->next)
      node->next->previous = node->previous;
    else
      last = node->previous;
    if(node->previous)
      node->previous->next = node->next;
    else
      first = node->next;
    --size;
    node->next = firstFree;
    firstFree = node;

    // remove from hash map
    *node->previousNextInTable = node->nextInTable;
  }

  void clear()
  {
    if(table)
    {
      if(size < tableSize / 2)
      {
        for(Node* i = first; i; i = i->next)
          *(i->previousNextInTable) = 0;
      }
      else
        for(Node** i = table, ** end = table + tableSize; i < end; ++i)
          *i = 0;
    }
    if(first)
    {
      last->next = firstFree;
      firstFree = first;
      first = last = 0;
      size = 0;
    }
  }

  Node* find(const K& key)
  {
    if(table)
    {
      unsigned int keyNum = (unsigned int)key;
      for(Node* i = table[keyNum  % tableSize]; i; i = i->nextInTable)
        if(i->keyNum == keyNum && i->key == key)
          return i;
    }
    return 0;
  }

  const Node* find(const K& key) const
  {
    if(table)
    {
      unsigned int keyNum = (unsigned int)key;
      for(Node* i = table[keyNum  % tableSize]; i; i = i->nextInTable)
        if(i->keyNum == keyNum && i->key == key)
          return i;
    }
    return 0;
  }

  T lookup(const K& key) const
  {
    if(table)
    {
      unsigned int keyNum = (unsigned int)key;
      for(Node* i = table[keyNum  % tableSize]; i; i = i->nextInTable)
        if(i->keyNum == keyNum && i->key == key)
          return i->data;
    }
    return T();
  }

  inline Node* getFirst() {return first;}
  inline const Node* getFirst() const {return first;}
  inline Node* getLast() {return last;}
  inline const Node* getLast() const {return last;}

  inline unsigned int getSize() const {return size;}

  inline bool isEmpty() const {return first == 0;}

private:
  Node* first;
  Node* last;
  unsigned int size;
  Node* firstFree;
  Node** table;

  enum Size
  {
    tableSize = 32,
  };
};
