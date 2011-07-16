
#pragma once

#include "Tools/String.h"
#include "Tools/List.h"
#include "Tools/Scope.h"

class Namespace;

class Statement : public Scope::Object
{
public:
  Statement(Scope& scope) : Scope::Object(scope) {}

  virtual void execute(Namespace& space) = 0;
};

class BlockStatement : public Statement
{
public:
  List<Statement*> statements;

  BlockStatement(Scope& scope) : Statement(scope) {}

private:
  virtual void execute(Namespace& space);
};

class AssignStatement : public Statement
{
public:
  AssignStatement(Scope& scope) : Statement(scope), value(0) {}

  String variable;
  Statement* value;

private:
  virtual void execute(Namespace& space);
};

class BinaryStatement : public Statement
{
public:
  BinaryStatement(Scope& scope) : Statement(scope) {}

  Statement* leftOperand;
  Statement* rightOperand;

private:
  virtual void execute(Namespace& space);
};

class StringStatement : public Statement
{
public:
  StringStatement(Scope& scope) : Statement(scope) {}

  String value;

private:
  virtual void execute(Namespace& space);
};

class ReferenceStatement : public Statement
{
public:
  ReferenceStatement(Scope& scope) : Statement(scope) {}

  String variable;

private:
  virtual void execute(Namespace& space);
};
