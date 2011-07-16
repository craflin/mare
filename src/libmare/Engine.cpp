
#include <cassert>

#include "Engine.h"
#include "Namespace.h"
#include "Parser.h"

bool Engine::load(const String& file)
{
  if(currentSpace)
    return false;
  Parser parser(*this);
  Statement* root = parser.parse(file, errorHandler, errorUserData);
  if(!root)
    return false;
  currentSpace = new Namespace(*this, 0, this, root, 0, false);
  assert(currentSpace);
  return true;
}

void Engine::error(const String& message)
{
  errorHandler(errorUserData, -1, message);
}

bool Engine::enterKey(const String& key, bool allowInheritance)
{
  Namespace* subSpace = currentSpace->enterKey(key, allowInheritance);
  if(subSpace)
  {
    currentSpace = subSpace;
    return true;
  }
  return false;
}

void Engine::enterUnnamedKey()
{
  Namespace* subSpace = currentSpace->enterUnnamedKey();
  assert(subSpace);
  currentSpace = subSpace;
}

void Engine::enterDefaultKey(const String& key)
{
  Namespace* subSpace = currentSpace->enterDefaultKey(key);
  assert(subSpace);
  currentSpace = subSpace;
}

bool Engine::resolveScript(const String& key, Namespace*& result)
{
  for(Namespace* space = currentSpace->getParent(); space; space = space->getParent())
    if(space->resolveScript(key, result))
      return true;
  return false;
}

bool Engine::leaveKey()
{
  if(!currentSpace->getParent())
    return false;
  currentSpace = currentSpace->getParent();
  assert(currentSpace);
  return true;
}

void Engine::getKeys(List<String>& keys)
{
  currentSpace->getKeys(keys);
}

void Engine::getKeys(const String& key, List<String>& keys, bool allowInheritance)
{
  if(enterKey(key, allowInheritance))
  {
    currentSpace->getKeys(keys);
    leaveKey();
  }
}

String Engine::getFirstKey()
{
  return currentSpace->getFirstKey();
}

String Engine::getFirstKey(const String& key, bool allowInheritance)
{
  if(enterKey(key, allowInheritance))
  {
    String result = currentSpace->getFirstKey();
    leaveKey();
    return result;
  }
  return String();
}

void Engine::addDefaultKey(const String& key)
{
  currentSpace->addKeyRaw(key, 0);
}

void Engine::addDefaultKey(const String& key, const String& value)
{
  enterDefaultKey(key);
  currentSpace->addResolvableKey(value, String());
  leaveKey();
}

void Engine::setDefaultKey(const String& key)
{
  currentSpace->setKeyRaw(key);
}

void Engine::addResolvableKey(const String& key, const String& value)
{
  currentSpace->addResolvableKey(key, value);
}
/*
void Engine::resetKey()
{
  currentSpace->reset();
}
*/
void Engine::pushKey()
{
  stashedKeys.append(currentSpace);
}

bool Engine::popKey()
{
  if(stashedKeys.isEmpty())
    return false;
  currentSpace = stashedKeys.getLast()->data;
  stashedKeys.removeLast();
  return true;
}


