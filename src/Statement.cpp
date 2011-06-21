
#include "Statement.h"
#include "Namespace.h"
#include "Engine.h"

void ValueStatement::execute(Namespace& space)
{
  if(executing)
    return; // cycling references
  executing = true;
  statement->execute(space);
  executing = false;
}

void StringValueStatement::execute(Namespace& space)
{
  if(executing)
    return; // cycling references
  executing = true;
  space.addVariable(value, 0);
  executing = false;
}

void BlockStatement::execute(Namespace& space)
{
  for(List<Statement*>::Node* i = statements.getFirst(); i; i = i->getNext())
    i->data->execute(space);
}

void AssignStatement::execute(Namespace& space)
{
  space.addVariable(variable, value);
}

void BinaryStatement::execute(Namespace& space)
{
  leftOperand->execute(space);
  rightOperand->execute(space);
}

void StringStatement::execute(Namespace& space)
{
  space.addVariable(value, 0);
}

void ReferenceStatement::execute(Namespace& space)
{
  Statement* statement = space.getEngine().resolveReference(variable);
  if(statement)
    statement->execute(space);
}
