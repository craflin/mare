
#include <cassert>

#include "Engine.h"
#include "Namespace.h"
#include "Parser.h"

bool Engine::load(const String& file, ErrorHandler errorHandler, void* userData)
{
  if(currentSpace)
    return false;
  Parser parser(*this);
  Script* root = parser.parse(file, errorHandler, userData);
  if(!root)
    return false;
  currentSpace = new Namespace(*this, 0, this, root, String());
  assert(currentSpace);
  return true;
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

bool Engine::resolveScript(const String& key, Script*& script)
{
  for(Namespace* space = currentSpace->getParent(); space; space = space->getParent())
    if(space->resolveScript(key, script))
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

void Engine::addDefaultKey(const String& key, const String& value)
{
  return currentSpace->addKeyRaw(key, value);
}

void Engine::addDefaultKey(const String& key)
{
  return currentSpace->addKeyRaw(key, 0);
}

void Engine::pushState()
{
  states.append(currentSpace);
}

bool Engine::popState()
{
  if(states.isEmpty())
    return false;
  currentSpace = states.getLast()->data;
  states.removeLast();
  return true;
}


