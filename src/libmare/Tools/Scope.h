
#pragma once

class Scope
{
public:
  class Object
  {
  public:
    Scope& scope;

  public:
    Object(Scope& scope);

    virtual ~Object();

  private:
    Object* next;
    Object* previous;
  };

  Scope() : first(0) {}

  virtual ~Scope();

  Scope& operator=(const Scope& other);

private:
  Object* first;
};

