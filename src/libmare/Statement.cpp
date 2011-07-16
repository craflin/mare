
#include <cassert>

#include "Statement.h"
#include "Namespace.h"
#include "Engine.h"

void BlockStatement::execute(Namespace& space)
{
  for(List<Statement*>::Node* i = statements.getFirst(); i; i = i->getNext())
    i->data->execute(space);
}

void AssignStatement::execute(Namespace& space)
{
  space.addKey(variable, value);
}

void BinaryStatement::execute(Namespace& space)
{
  leftOperand->execute(space);
  rightOperand->execute(space);
}

void StringStatement::execute(Namespace& space)
{
  space.addKey(value, 0);
}

void ReferenceStatement::execute(Namespace& space)
{
  Namespace* ref;
  if(space.getEngine().resolveScript(variable, ref))
    if(ref)
    {
      assert(!ref->compiling);
      ref->compiling = true;
      ref->statement->execute(space);
      ref->compiling = false;
    }
}
