
#pragma once

#include "Tools/String.h"
#include "Tools/List.h"
#include "Tools/Scope.h"
#include "Namespace.h"

class Engine : public Scope
{
public:

  typedef void (*ErrorHandler)(void* userData, unsigned int line, const String& message);

  bool load(const String& file, ErrorHandler errorHandler, void* userData);

  bool enter(const String& key, bool allowInheritance = true);
  void enterNew();
  //inline String getNode() const {return currentNode.name;}
  //String getParentNode() const;
  ValueStatement* resolveReference(const String& key);

  bool leave();
  void getKeys(List<String>& keys);
  void getKeys(const String& key, List<String>& keys, bool allowInheritance = true);
  String getFirstKey();
  void addDefaultVariable(const String& key, const String& value);

private:
  List<Namespace> namespaces;
};
