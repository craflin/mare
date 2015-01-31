
#pragma once

#include "Tools/Map.h"
#include "Tools/Scope.h"
#include "Tools/Word.h"
#include "Token.h"

class Engine;
class Statement;

class Namespace : public Scope, public Scope::Object
{
public:
  Namespace(Scope& scope, Namespace* parent, Engine* engine, Statement* statement, Namespace* next, unsigned int flags) : Scope::Object(scope), parent(parent), defaultStatement(0), statement(statement), next(next), engine(engine), flags(flags) {}
  
  inline Namespace* getParent() {return parent;}
  bool resolveScript2(const String& name, Word*& word, Namespace*& result);
  bool resolveScript2(const String& name, Namespace* excludeStatements, Word*& word, Namespace*& result);
  Namespace* enterKey(const String& name, bool allowInheritance);
  Namespace* enterUnnamedKey(Statement* statement);
  Namespace* enterNewKey(const String& name);
  String getKeyOrigin(const String& key);
  void getKeys(List<String>& keys);
  void getText(List<String>& text);
  void appendKeys(String& output);
  String getFirstKey();
  String getMareDir() const;
  inline Engine& getEngine() {return *engine;}

  void addKey(const String& key, unsigned int flags, Statement* value, Token::Id operation = Token::assignment);
  void addKeyRaw(const Word& key, Statement* value, Token::Id operation = Token::assignment);
  void setKeyRaw(const Word& key);
  void removeAllKeys();
  void removeKey(const String& key);
  void removeKeyRaw(const String& key);
  void removeKeysRaw(Namespace& space);
  bool compareKeys(Namespace& space, bool& result);
  bool versionCompareKeys(Namespace& space, int& result);

  void addDefaultStatement(Statement* statement);
  void addDefaultKey(const String& key);
  void addDefaultKey(const String& key, unsigned int flags, const String& value);
  void addDefaultKey(const String& key, unsigned int flags, const Map<String, String>& value);

private:
  enum Flags
  {
    inheritedFlag = (1 << 1),
    compiledFlag = (1 << 2),
    compilingFlag = (1 << 3),
    unnamedFlag = (1 << 4),
    textModeFlag = (1 << 5),
  };

  Namespace* parent;
  Statement* defaultStatement;
  Statement* statement;
  Namespace* next;
  Engine* engine;
  unsigned int flags; 
  Map<Word, Namespace*> variables;

  void compile();
  String evaluateString(const String& string) const;

  friend class Engine;
  friend class ReferenceStatement; // temporary hack
};
