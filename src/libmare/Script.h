
#pragma once

#include "Tools/String.h"
#include "Tools/Scope.h"

class Namespace;
class Statement;

class Script : public Scope::Object
{
public:
  Statement* statement;

  Script(Scope& scope, Statement* statement, Script* next = 0) : Scope::Object(scope), statement(statement), next(next), executing(false) {}

  bool execute(Namespace& space);

  Script* next; /**< A script that was replaced by this one */
  bool executing;
};
