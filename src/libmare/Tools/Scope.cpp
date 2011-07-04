
#include <cassert>

#include "Scope.h"

Scope::Object::Object(Scope& scope) : scope(scope), previous(0)
{
  if((next = scope.first))
    next->previous = this;
  scope.first = this;
}

Scope::Object::~Object()
{
  if(next)
    next->previous = previous;
  if(previous)
    previous->next = next;
  else
    scope.first = next;
}

Scope::~Scope()
{
  while(first)
    delete first;
}

Scope& Scope::operator=(const Scope& other)
{
  while(first)
    delete first;
  assert(other.first == 0);
  return *this;
}
