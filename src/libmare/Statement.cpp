
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
  /*
  if(variable == "testa")
  {
    int k = 42;
  }
  if(variable == "testb")
  {
    int k = 42;
  }
  */
  space.addKey(variable, value);
}

void BinaryStatement::execute(Namespace& space)
{
  switch(operation)
  {
  case Token::plus:
  case Token::plusAssignment:
    leftOperand->execute(space);
    rightOperand->execute(space);
    break;
  case Token::minus:
  case Token::minusAssignment:
    {
      leftOperand->execute(space);
      Namespace* rightSpace = new Namespace(space.getEngine(), &space, &space.getEngine(), rightOperand, 0, false);
      space.removeKeys(*rightSpace);
      delete rightSpace;
    }
    break;
  case Token::and_:
  case Token::or_:
  case Token::equal:
  case Token::notEqual:
  case Token::greaterThan:
  case Token::lowerThan:
  case Token::greaterEqualThan:
  case Token::lowerEqualThan:
    {
      Namespace* leftSpace = new Namespace(space.getEngine(), &space, &space.getEngine(), leftOperand, 0, false);
      Namespace* rightSpace = new Namespace(space.getEngine(), &space, &space.getEngine(), rightOperand, 0, false);
      bool result = false;
      switch(operation)
      {
      case Token::and_:
        result = !leftSpace->getFirstKey().isEmpty() && !rightSpace->getFirstKey().isEmpty();
        break;
      case Token::or_:
        result = !!leftSpace->getFirstKey().isEmpty() || !rightSpace->getFirstKey().isEmpty();
        break;
      case Token::equal:
        leftSpace->compareKeys(*rightSpace, result);
        break;
      case Token::notEqual:
        if(leftSpace->compareKeys(*rightSpace, result))
          result = !result;
        break;
      case Token::greaterThan:
        {
          int val;
          if(leftSpace->versionCompareKeys(*rightSpace, val))
            result = val > 0;
        }
        break;
      case Token::lowerThan:
        {
          int val;
          if(leftSpace->versionCompareKeys(*rightSpace, val))
            result = val < 0;
        }
        break;
      case Token::greaterEqualThan:
        {
          int val;
          if(leftSpace->versionCompareKeys(*rightSpace, val))
            result = val >= 0;
        }
        break;
      case Token::lowerEqualThan:
        {
          int val;
          if(leftSpace->versionCompareKeys(*rightSpace, val))
            result = val <= 0;
        }
        break;
      default:
        assert(false);
        break;
      }
      delete leftSpace;
      delete rightSpace;
      if(result)
        space.addKey("true", 0);
    }
    break;

  default:
    assert(false);
    break;
  }
}

void StringStatement::execute(Namespace& space)
{
  space.addKey(value, 0);
}

void ReferenceStatement::execute(Namespace& space)
{
  Namespace* ref;
  if(space.getEngine().resolveScript(variable, ref))
    if(ref && ref->statement)
    {
      assert(!ref->compiling);
      ref->compiling = true;
      ref->statement->execute(space);
      ref->compiling = false;
    }
}

void IfStatement::execute(Namespace& space)
{
  Namespace* condSpace = new Namespace(space.getEngine(), &space, &space.getEngine(), condition, 0, false);
  bool cond = !condSpace->getFirstKey().isEmpty();
  delete condSpace;
  if(cond)
    thenStatements->execute(space);
  else if(elseStatements)
    elseStatements->execute(space);
}

void ConditionalStatement::execute(Namespace& space)
{
  Namespace* condSpace = new Namespace(space.getEngine(), &space, &space.getEngine(), condition, 0, false);
  bool cond = !condSpace->getFirstKey().isEmpty();
  delete condSpace;
  if(cond)
    thenStatements->execute(space);
  else if(elseStatements)
    elseStatements->execute(space);
}

void UnaryStatement::execute(Namespace& space)
{
  switch(operation)
  {
  case Token::not_:
    {
      Namespace* opSpace = new Namespace(space.getEngine(), &space, &space.getEngine(), operand, 0, false);
      bool result = opSpace->getFirstKey().isEmpty();
      delete opSpace;
      if(result)
        space.addKey("true", 0);
    }
    break;
  default:
    assert(false);
    break;
  }
}
