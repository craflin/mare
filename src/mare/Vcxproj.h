
#pragma once

#include "../libmare/Tools/Map.h"
#include "../libmare/Tools/String.h"

class Engine;

class Vcxproj
{
public:

  Vcxproj(Engine& engine, int version) : engine(engine), version(version) {}

  bool generate(const Map<String, String>& userArgs);

private:
  Engine& engine;
  int version; /**< Version of the vcxproj file format e.g. 2010 */
};

