
#pragma once

#include "Tools/String.h"
#include "Tools/List.h"
#include "Tools/Scope.h"
#include "Namespace.h"

class Script;

class Engine : public Scope
{
public:

  typedef void (*ErrorHandler)(void* userData, unsigned int line, const String& message);

  Engine() : currentSpace(0) {}

  bool load(const String& file, ErrorHandler errorHandler, void* userData);

  bool enter(const String& key, bool allowInheritance = true);
  void enterNew();

  bool resolveScript(const String& key, Script*& script);

  bool leave();
  void getKeys(List<String>& keys);
  void getKeys(const String& key, List<String>& keys, bool allowInheritance = true);
  String getFirstKey();
  void addKey(const String& key, const String& value);
  void addKey(const String& key);

private:
  Namespace* currentSpace;
};
