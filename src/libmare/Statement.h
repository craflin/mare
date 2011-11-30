
#pragma once

#include "Tools/String.h"
#include "Tools/List.h"
#include "Tools/Scope.h"
#include "Token.h"

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
  AssignStatement(Scope& scope) : Statement(scope), flags(0), value(0) {}

  String variable;
  unsigned int flags;
  Statement* value;

private:
  virtual void execute(Namespace& space);
};

class RemoveStatement : public Statement
{
public:
  RemoveStatement(Scope& scope) : Statement(scope) {}

  String variable;

private:
  virtual void execute(Namespace& space);
};

class BinaryStatement : public Statement
{
public:
  BinaryStatement(Scope& scope) : Statement(scope) {}

  Token::Id operation;
  Statement* leftOperand;
  Statement* rightOperand;

private:
  virtual void execute(Namespace& space);
};

class UnaryStatement : public Statement
{
public:
  UnaryStatement(Scope& scope) : Statement(scope) {}

  Token::Id operation;
  Statement* operand;

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

/** "if ... then ... else ..." and " ... ? ... : ..." */
class IfStatement : public Statement
{
public:
  IfStatement(Scope& scope) : Statement(scope), elseStatements(0) {}

  Statement* condition;
  Statement* thenStatements;
  Statement* elseStatements;

private:
  virtual void execute(Namespace& space);
};
