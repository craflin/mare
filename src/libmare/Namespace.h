
#pragma once

#include "Tools/Map.h"
#include "Tools/Scope.h"

class Engine;
class Script;

class Namespace : public Scope, public Scope::Object
{
public:
  Namespace(Scope& scope, Namespace* parent, Engine* engine, Script* script, bool inherited) : Scope::Object(scope), parent(parent), engine(engine), script(script), inherited(inherited), compiled(false), unnamedSpace(0) {}
  
  inline Namespace* getParent() {return parent;}
  bool resolveScript(const String& name, Script*& script);
  Namespace* enterKey(const String& name, bool allowInheritance);
  Namespace* enterUnnamedKey();
  Namespace* enterDefaultKey(const String& name);
  void getKeys(List<String>& keys);
  String getFirstKey();
  inline Engine& getEngine() {return *engine;}

  void addKey(const String& key, Script* value);
  void addKeyRaw(const String& key, Script* value);
  void setKeyRaw(const String& key);

  void addResolvableKey(const String& key, const String& value);

  void reset();

private:
  Namespace* parent;
  Engine* engine;
  Script* script;
  bool inherited;
  bool compiled;
  Map<String, Script*> variables;
  Map<String, Namespace*> spaces;
  Namespace* unnamedSpace;

  bool compile();
  String evaluateString(const String& string);
};
