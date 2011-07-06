
#include "Script.h"
#include "Statement.h"

bool Script::execute(Namespace& space)
{
  if(executing)
    return false; // cycling references
  executing = true;
  if(statement)
    statement->execute(space);
  executing = false;
  return true;
}
