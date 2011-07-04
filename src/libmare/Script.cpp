
#include "Script.h"
#include "Statement.h"
#include "Namespace.h"

bool StatementScript::execute(Namespace& space)
{
  if(executing)
    return false; // cycling references
  executing = true;
  statement.execute(space);
  executing = false;
  return true;
}

bool StringScript::execute(Namespace& space)
{
  if(executing)
    return false; // cycling references
  executing = true;
  space.addKey(value, 0);
  executing = false;
  return true;
}
