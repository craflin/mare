
#include "Engine.h"
#include "Namespace.h"
#include "Parser.h"

bool Engine::load(const String& file, ErrorHandler errorHandler, void* userData)
{
  if(!namespaces.isEmpty())
    return false;
  Parser parser(*this);
  Script* root = parser.parse(file, errorHandler, userData);
  if(!root)
    return false;
  namespaces.append(Namespace(this, root, String()));
  return true;
}

bool Engine::enter(const String& key, bool allowInheritance)
{
  List<Namespace>::Node* i = namespaces.getLast();
  Script* script;
  if(i->data.resolve(key, script))
    goto success;
  if(allowInheritance)
    for(i = i->getPrevious(); i; i = i->getPrevious())
      if(i->data.resolve(key, script))
        goto success;
  return false;
success:
  namespaces.append(Namespace(this, script, key));
  return true;
}

void Engine::enterNew()
{
  namespaces.append(Namespace(this, 0, String()));
}

Script* Engine::resolveReference(const String& key)
{
  Script* script;
  for(List<Namespace>::Node* i = namespaces.getLast()->getPrevious(); i; i = i->getPrevious())
    if(i->data.resolve(key, script))
      return script;
  return 0;
}

bool Engine::leave()
{
  if(namespaces.getSize() <= 1)
    return false;
  namespaces.removeLast();
  return true;
}

void Engine::getKeys(List<String>& keys)
{
  namespaces.getLast()->data.getKeys(keys);
}

void Engine::getKeys(const String& key, List<String>& keys, bool allowInheritance)
{
  if(enter(key, allowInheritance))
  {
    getKeys(keys);
    leave();
  }
  else
    keys.clear();
}

String Engine::getFirstKey()
{
  return namespaces.getLast()->data.getFirstKey();
}

void Engine::addDefaultVariable(const String& key, const String& value)
{
  return namespaces.getLast()->data.addDefaultVariableRaw(key, value);
}
