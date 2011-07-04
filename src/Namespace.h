
#pragma once

#include "Tools/Map.h"
#include "Tools/Scope.h"

class Engine;
class Script;

class Namespace : public Scope, public Scope::Object
{
public:
  Namespace(Scope& scope, Namespace* parent, Engine* engine, Script* script) : Scope::Object(scope), parent(parent), engine(engine), script(script), compiled(false), temporarySpace(0) {}
  
  inline Namespace* getParent() {return parent;}
  bool resolveScript(const String& name, Script*& script);
  Namespace* enter(const String& name, bool allowInheritance);
  Namespace* enterTemporary();
  void getKeys(List<String>& keys);
  String getFirstKey();
  inline Engine& getEngine() {return *engine;}

  void addKey(const String& key, Script* value);
  void addKeyRaw(const String& key, Script* value);
  void addKeyRaw(const String& key, const String& value);

  void rest();

private:
  class Variable
  {
  public:
    Script* script;
    Namespace* space;
    bool inherited;

    Variable(Script* script, bool inherited = false) : script(script), space(0), inherited(inherited) {}
  };

  Namespace* parent;
  Engine* engine;
  Script* script;
  bool compiled;
  Map<String, Variable> variables;
  Namespace* temporarySpace;

  void compile();
  String evaluateString(const String& string);
};
