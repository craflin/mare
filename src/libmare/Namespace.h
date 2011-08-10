
#pragma once

#include "Tools/Map.h"
#include "Tools/Scope.h"
#include "Tools/Word.h"

class Engine;
class Statement;

class Namespace : public Scope, public Scope::Object
{
public:
  Namespace(Scope& scope, Namespace* parent, Engine* engine, Statement* statement, Namespace* next, bool inherited) : Scope::Object(scope), parent(parent), defaultStatement(0), statement(statement), next(next), engine(engine), inherited(inherited), compiled(false), compiling(false) {}
  
  inline Namespace* getParent() {return parent;}
  bool resolveScript2(const String& name, Word*& word, Namespace*& space);
  Namespace* enterKey(const String& name, bool allowInheritance);
  Namespace* enterUnnamedKey(Statement* statement);
  Namespace* enterNewKey(const String& name);
  void getKeys(List<String>& keys);
  void appendKeys(String& output);
  String getFirstKey();
  inline Engine& getEngine() {return *engine;}

  void addKey(const String& key, Statement* value);
  void addKeyRaw(const Word& key, Statement* value);
  void setKeyRaw(const Word& key);
  void removeKey(const String& key);
  void removeKeyRaw(const String& key);
  void removeKeysRaw(Namespace& space);
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
  Map<Word, Namespace*> variables;

  bool compile();
  String evaluateString(const String& string);

  friend class Engine;
  friend class ReferenceStatement; // temporary hack
};
