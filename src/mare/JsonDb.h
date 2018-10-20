
#pragma once

#include "Tools/Map.h"
#include "Tools/String.h"

class Engine;
class File;

class JsonDb
{
public:

  JsonDb(Engine& engine) : engine(engine) {}

  bool generate(const Map<String, String>& userArgs);

private:

  static void writeJsonEscaped(File& f, const String& string);

  Engine& engine;
};
