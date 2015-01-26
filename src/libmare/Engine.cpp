
#include "Tools/Assert.h"
#include "Engine.h"
#include "Namespace.h"
#include "Parser.h"

bool Engine::load(const String& file)
{
  if(currentSpace)
    return false;
  Parser parser(*this);
  ASSERT(!rootStatement);
  rootStatement = parser.parse(file, errorHandler, errorUserData);
  if(!rootStatement)
    return false;
  currentSpace = new Namespace(*this, 0, this, 0, 0, 0);
  return true;
}

void Engine::error(const String& message)
{
  errorHandler(errorUserData, String(), -1, message);
}

bool Engine::hasKey(const String& key, bool allowInheritance)
{
  if(!currentSpace->enterKey(key, allowInheritance))
    return false;
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

void Engine::enterRootKey()
{
  Namespace* subSpace = currentSpace->enterUnnamedKey(rootStatement);
  ASSERT(subSpace);
  currentSpace = subSpace;
}

String Engine::getKeyOrigin(const String& key)
{
  return currentSpace->getKeyOrigin(key);
}

bool Engine::resolveScript(const String& key, Word*& word, Namespace*& result)
{
  for(Namespace* space = currentSpace->getParent(); space; space = space->getParent())
    if(space->resolveScript2(key, word, result))
      return true;
  return false;
}

bool Engine::resolveScript(const String& key, Namespace* excludeStatements, Word*& word, Namespace*& result)
{
  for(Namespace* space = currentSpace->getParent(); space; space = space->getParent())
    if(space->resolveScript2(key, excludeStatements, word, result))
      return true;
  return false;
}

bool Engine::leaveKey()
{
  if(!currentSpace->getParent())
    return false;
  Namespace* parent = currentSpace->getParent();
  if(currentSpace->flags & Namespace::unnamedFlag)
    delete currentSpace;
  currentSpace = parent;
  return true;
}

void Engine::getKeys(List<String>& keys)
{
  currentSpace->getKeys(keys);
}

void Engine::appendKeys(String& output)
{
  return currentSpace->appendKeys(output);
}

bool Engine::getKeys(const String& key, List<String>& keys, bool allowInheritance)
{
  if(enterKey(key, allowInheritance))
  {
    currentSpace->getKeys(keys);
    leaveKey();
    return true;
  }
  return false;
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

void Engine::getText(List<String>& text)
{
  currentSpace->getText(text);
}

bool Engine::getText(const String& key, List<String>& text, bool allowInheritance)
{
  if(enterKey(key, allowInheritance))
  {
    currentSpace->getText(text);
    leaveKey();
    return true;
  }
  return false;
}

void Engine::addDefaultKey(const String& key)
{
  currentSpace->addDefaultKey(key);
}

void Engine::addDefaultKey(const String& key, const String& value)
{
  currentSpace->addDefaultKey(key, Word::defaultFlag, value);
}

void Engine::addDefaultKey(const String& key, const Map<String, String>& value)
{
  currentSpace->addDefaultKey(key, Word::defaultFlag, value);
}

void Engine::addCommandLineKey(const String& key, const String& value)
{
  currentSpace->addDefaultKey(key, Word::commandLineFlag, value);
}

void Engine::setKey(const Word& key)
{
  currentSpace->setKeyRaw(key);
}

void Engine::pushAndLeaveKey()
{
  stashedKeys.append(currentSpace);
  currentSpace = currentSpace->getParent();
}

bool Engine::popKey()
{
  if(stashedKeys.isEmpty())
    return false;
  currentSpace = stashedKeys.getLast()->data;
  stashedKeys.removeLast();
  return true;
}
