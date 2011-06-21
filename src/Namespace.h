
#pragma once

#include "Tools/Map.h"
#include "Tools/Scope.h"

class Engine;
class ValueStatement;

class Namespace : public Scope
{
public:
  Namespace(Engine* engine, ValueStatement* statement, const String& name) : engine(engine), statement(statement), name(name), compiled(false) {}
  
  bool resolve(const String& name, ValueStatement*& statement);
  void getKeys(List<String>& keys);
  String getFirstKey();
  inline Engine& getEngine() {return *engine;}

  void addVariable(const String& key, ValueStatement* value);
  void addVariableRaw(const String& key, ValueStatement* value);
  void addDefaultVariableRaw(const String& key, const String& value);

private:
  Engine* engine;
  ValueStatement* statement;
  String name;
  bool compiled;
  Map<String, ValueStatement*> variables;

  void compile();
  String evaluateString(const String& string);
};
