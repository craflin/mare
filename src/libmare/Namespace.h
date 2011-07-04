
#pragma once

#include "Tools/Map.h"
#include "Tools/Scope.h"

class Engine;
class Script;

class Namespace : public Scope, public Scope::Object
{
public:
  Namespace(Scope& scope, Namespace* parent, Engine* engine, Script* script, const String& name) : Scope::Object(scope), parent(parent), engine(engine), script(script), compiled(false), unnamedSpace(0), name(name)/*, compiling(false)*/ {}
  
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
  void addKeyRaw(const String& key, const String& value);

  void reset();

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
  Namespace* unnamedSpace;
  String name; // for debugging
  //bool compiling; // for debugging?

  bool compile();
  String evaluateString(const String& string);
};
