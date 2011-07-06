
#pragma once

template <typename T> class List
{
public:
  class Node
  {
  public:
    T data;

    Node() {}
    Node(const T& data) : data(data) {}

    inline Node* getNext() {return next;}
    inline const Node* getNext() const {return next;}
    inline Node* getPrevious() {return previous;}
    inline const Node* getPrevious() const {return previous;}

  private:
    Node* next;
    Node* previous;

    friend class List;
  };

  List() : first(0), last(0), size(0), firstFree(0) {}

  ~List()
  {
    clear();
    for(Node* node = firstFree, * next; node; node = next)
    {
      next = node->next;
      delete node;
    }
  }

  List& operator=(const List& other)
  {
    clear();
    for(Node* i = other.first; i; i = i->next)
      append(i->data);
    return *this;
  }

  T& append(const T& data = T())
  {
    Node* node;
    if(firstFree)
    {
      node = firstFree;
      firstFree = firstFree->next;
      node->data = data;
    }
    else
      node = new Node(data);
    node->next = 0;
    if((node->previous = last))
      last->next = node;
    else
      first = node;
    last = node;
    ++size;
    return node->data;
  }

  T& prepend(const T& data = T())
  {
    Node* node;
    if(firstFree)
    {
      node = firstFree;
      firstFree = firstFree->next;
      node->data = data;
    }
    else
      node = new Node(data);
    node->previous = 0;
    if((node->next = first))
      first->previous = node;
    else
      last = node;
    first = node;
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

  inline void removeFirst() {remove(getFirst());}
  inline void removeLast() {remove(getLast());}

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

