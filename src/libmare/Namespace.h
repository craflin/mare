
#pragma once

#include "Tools/Map.h"
#include "Tools/Scope.h"

class Engine;
class Statement;

class Namespace : public Scope, public Scope::Object
{
public:
  Namespace(Scope& scope, Namespace* parent, Engine* engine, Statement* statement, Namespace* next, bool inherited) : Scope::Object(scope), parent(parent), statement(statement), next(next), engine(engine), inherited(inherited), compiled(false), compiling(false), unnamedSpace(0) {}
  
  inline Namespace* getParent() {return parent;}
  bool resolveScript(const String& name, Namespace*& space);
  Namespace* enterKey(const String& name, bool allowInheritance);
  Namespace* enterUnnamedKey();
  Namespace* enterDefaultKey(const String& name);
  void getKeys(List<String>& keys);
  String getFirstKey();
  inline Engine& getEngine() {return *engine;}

  void addKey(const String& key, Statement* value);
  void addKeyRaw(const String& key, Statement* value);
  void setKeyRaw(const String& key);
  void removeKeys(Namespace& space);
  bool compareKeys(Namespace& space, bool& result);
  bool versionCompareKeys(Namespace& space, int& result);

  void addResolvableKey(const String& key, const String& value);

  //void reset();

private:
  Namespace* parent;
  Statement* statement;
  Namespace* next;
  Engine* engine;
  bool inherited;
  bool compiled;
  bool compiling;
  Map<String, Namespace*> variables;
  Namespace* unnamedSpace;

  bool compile();
  String evaluateString(const String& string);

  friend class ReferenceStatement; // temporary hack
};
