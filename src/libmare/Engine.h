
#pragma once

#include "Tools/String.h"
#include "Tools/List.h"
#include "Tools/Map.h"
#include "Tools/Scope.h"

class Namespace;

class Engine : public Scope
{
public:

  typedef void (*ErrorHandler)(void* userData, int line, const String& message);

  Engine(ErrorHandler errorHandler, void* userData) : errorHandler(errorHandler), errorUserData(userData), currentSpace(0) {}

  bool load(const String& file);
  void error(const String& message);

  bool enterKey(const String& key, bool allowInheritance = true);
  void enterUnnamedKey();
  void enterNewKey(const String& key);

  bool leaveKey();
  //bool leaveUnnamedKey();
  void getKeys(List<String>& keys);
  void getKeys(const String& key, List<String>& keys, bool allowInheritance = true);
  String getFirstKey();
  String getFirstKey(const String& key, bool allowInheritance = true);

  void setKey(const String& key);

  void addDefaultKey(const String& key);
  void addDefaultKey(const String& key, const String& value);
  //void addDefaultKey(const String& key, const String& value, const String& subValue);
  void addDefaultKey(const String& key, const Map<String, String>& value);

  void pushKey();
  bool popKey();

private:
  ErrorHandler errorHandler;
  void* errorUserData;
  Namespace* currentSpace;
  List<Namespace*> stashedKeys;

  bool resolveScript(const String& key, Namespace*& space);
  
  friend class ReferenceStatement;
  friend class Namespace;
};
