
#pragma once

#include "../libmare/Tools/List.h"
#include "../libmare/Tools/Map.h"
#include "../libmare/Tools/String.h"

class Engine;

class Builder
{
public:

  Builder(Engine& engine) : engine(engine) {}

  bool build(const Map<String, String>& userArgs);

private:
  Engine& engine;
  List<String> inputConfigs;
  List<String> inputTargets;

  bool buildFile();
  bool buildConfigurations();
  bool buildConfiguration(const String& configuration);
  bool buildTargets();
};
