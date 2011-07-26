
#include "Tools/Assert.h"
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
  Namespace* subSpace = currentSpace->enterUnnamedKey(0);
  ASSERT(subSpace);
  currentSpace = subSpace;
}

void Engine::enterNewKey(const String& key)
{
  Namespace* subSpace = currentSpace->enterNewKey(key);
  ASSERT(subSpace);
  currentSpace = subSpace;
}

bool Engine::resolveScript(const String& key, Namespace*& result)
{
  if(currentSpace->getParent())
    return currentSpace->getParent()->resolveScript2(key, result);
  return false;
}

bool Engine::leaveKey()
{
  if(!currentSpace->getParent())
    return false;
  currentSpace = currentSpace->getParent();
  return true;
}

bool Engine::leaveUnnamedKey()
{
  if(!currentSpace->getParent())
    return false;
  Namespace* toDelete = currentSpace;
  currentSpace = currentSpace->getParent();
  delete toDelete;
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
  currentSpace->addDefaultKey(key);
}

void Engine::addDefaultKey(const String& key, const String& value)
{
  currentSpace->addDefaultKey(key, value);
}
/*
void Engine::addDefaultKey(const String& key, const String& value, const String& subValue)
{
  Map<String, String> values;
  values.append(value, subValue);
  currentSpace->addResolvableKey(key, values);
}
*/
void Engine::addDefaultKey(const String& key, const Map<String, String>& value)
{
  currentSpace->addDefaultKey(key, value);
}

void Engine::setKey(const String& key)
{
  currentSpace->setKeyRaw(key);
}

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


