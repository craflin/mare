
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

    friend class Map;
  };

  Map() : first(0), last(0), size(0), firstFree(0) {}

  ~Map()
  {
    clear();
    for(Node* node = firstFree, * next; node; node = next)
    {
      next = node->next;
      delete node;
    }
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
      first = node;
    last = node;
    ++size;
    return node->data;
  }

  void remove(Node* node)
  {
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
  }

  void clear()
  {
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
    // TODO: use hash map
    for(Node* node = first; node; node = node->next)
      if(node->key == key)
        return node;
    return 0;
  }

  const Node* find(const K& key) const
  {
    // TODO: use hash map
    for(const Node* node = first; node; node = node->next)
      if(node->key == key)
        return node;
    return 0;
  }

  T lookup(const K& key)
  {
    // TODO: use hash map
    for(Node* node = first; node; node = node->next)
      if(node->key == key)
        return node->data;
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
};

