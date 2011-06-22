
#pragma once

#include "Tools/Map.h"
#include "Tools/Scope.h"

class Engine;
class Script;

class Namespace : public Scope
{
public:
  Namespace(Engine* engine, Script* script, const String& name) : engine(engine), script(script), name(name), compiled(false) {}
  
  bool resolve(const String& name, Script*& script);
  void getKeys(List<String>& keys);
  String getFirstKey();
  inline Engine& getEngine() {return *engine;}

  void addVariable(const String& key, Script* value);
  void addVariableRaw(const String& key, Script* value);
  void addDefaultVariableRaw(const String& key, const String& value);

private:
  Engine* engine;
  Script* script;
  String name;
  bool compiled;
  Map<String, Script*> variables;

  void compile();
  String evaluateString(const String& string);
};
