

#include "Tools/Assert.h"
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
  space.addKey(variable, flags, value, operation);
}

void RemoveStatement::execute(Namespace& space)
{
  space.removeKey(variable);
}

void BinaryStatement::execute(Namespace& space)
{
  switch(operation)
  {
  case Token::plus:
    leftOperand->execute(space);
    rightOperand->execute(space);
    break;
  case Token::minus:
    {
      leftOperand->execute(space);
      Namespace* rightSpace = new Namespace(space.getEngine(), &space, &space.getEngine(), rightOperand, 0, 0);
      space.removeKeysRaw(*rightSpace);
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
      Namespace* leftSpace = new Namespace(space.getEngine(), &space, &space.getEngine(), leftOperand, 0, 0);
      Namespace* rightSpace = new Namespace(space.getEngine(), &space, &space.getEngine(), rightOperand, 0, 0);
      bool result = false;
      switch(operation)
      {
      case Token::and_:
        result = !leftSpace->getFirstKey().isEmpty() && !rightSpace->getFirstKey().isEmpty();
        break;
      case Token::or_:
        result = !leftSpace->getFirstKey().isEmpty() || !rightSpace->getFirstKey().isEmpty();
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
        ASSERT(false);
        break;
      }
      delete leftSpace;
      delete rightSpace;
      if(result)
        space.addKeyRaw(Word("true", 0), 0);
    }
    break;

  default:
    ASSERT(false);
    break;
  }
}

void StringStatement::execute(Namespace& space)
{
  space.addKey(value, 0, 0);
}

void ReferenceStatement::execute(Namespace& space)
{
  Word* word;
  Namespace* ref;
  if(space.getEngine().resolveScript(variable, word, ref))
    if(ref && ref->statement)
    {
      ASSERT(!(ref->flags & Namespace::compilingFlag));
      ref->flags |= Namespace::compilingFlag;
      ref->statement->execute(space);
      ref->flags &= ~Namespace::compilingFlag;
    }
}

void IfStatement::execute(Namespace& space)
{
  Namespace* condSpace = new Namespace(space.getEngine(), &space, &space.getEngine(), condition, 0, 0);
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
      Namespace* opSpace = new Namespace(space.getEngine(), &space, &space.getEngine(), operand, 0, 0);
      bool result = opSpace->getFirstKey().isEmpty();
      delete opSpace;
      if(result)
        space.addKeyRaw(Word("true", 0), 0);
    }
    break;
  default:
    ASSERT(false);
    break;
  }
}
