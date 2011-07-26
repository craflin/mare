
#pragma once

#include "Tools/Map.h"
#include "Tools/Scope.h"

class Engine;
class Statement;

class Namespace : public Scope, public Scope::Object
{
public:
  Namespace(Scope& scope, Namespace* parent, Engine* engine, Statement* statement, Namespace* next, bool inherited) : Scope::Object(scope), parent(parent), defaultStatement(0), statement(statement), next(next), engine(engine), inherited(inherited), compiled(false), compiling(false), inheritedSpaces(0) {}
  
  inline Namespace* getParent() {return parent;}
  bool resolveScript2(const String& name, Namespace*& space);
  Namespace* enterKey(const String& name, bool allowInheritance);
  Namespace* enterUnnamedKey(Statement* statement);
  Namespace* enterNewKey(const String& name);
  void getKeys(List<String>& keys);
  String getFirstKey();
  inline Engine& getEngine() {return *engine;}

  void addKey(const String& key, Statement* value); // TODO: rename to registerKey?
  void addKeyRaw(const String& key, Statement* value); // TODO: rename to registerKeyRaw?
  void setKeyRaw(const String& key); // TODO: remove this and add something like clearKeys()
  void removeKeys(Namespace& space);
  bool compareKeys(Namespace& space, bool& result);
  bool versionCompareKeys(Namespace& space, int& result);

  void addDefaultStatement(Statement* statement);
  void addDefaultKey(const String& key);
  void addDefaultKey(const String& key, const String& value);
  void addDefaultKey(const String& key, const Map<String, String>& value);

private:
  Namespace* parent;
  Statement* defaultStatement;
  Statement* statement;
  Namespace* next;
  Engine* engine;
  bool inherited;
  bool compiled;
  bool compiling;
  Map<String, Namespace*> variables;

  Namespace* inheritedSpaces;

  bool compile();
  String evaluateString(const String& string);

  friend class Engine;
  friend class ReferenceStatement; // temporary hack
};
