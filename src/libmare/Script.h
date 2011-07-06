
#pragma once

#include "Tools/String.h"
#include "Tools/Scope.h"

class Namespace;
class Statement;

class Script : public Scope::Object
{
public:
  Statement* statement;

  Script(Scope& scope, Statement* statement) : Scope::Object(scope), executing(false), statement(statement) {}

  bool execute(Namespace& space);

private:
  bool executing;
};
