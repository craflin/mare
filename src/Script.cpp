
#include "Script.h"
#include "Statement.h"
#include "Namespace.h"

void StatementScript::execute(Namespace& space)
{
  if(executing)
    return; // cycling references
  executing = true;
  statement.execute(space);
  executing = false;
}

void StringScript::execute(Namespace& space)
{
  if(executing)
    return; // cycling references
  executing = true;
  space.addVariable(value, 0);
  executing = false;
}
