
#pragma once

#include "../libmare/Tools/List.h"
#include "../libmare/Tools/Map.h"
#include "../libmare/Tools/String.h"

class Engine;

class Builder
{
public:

  Builder(Engine& engine, bool showDebug, bool clean, bool rebuild) : engine(engine), showDebug(showDebug), clean(clean), rebuild(rebuild) {}

  bool build(const Map<String, String>& userArgs);

private:
  Engine& engine;
  bool showDebug;
  bool clean;
  bool rebuild;

  List<String> inputPlatforms;
  List<String> inputConfigs;
  List<String> inputTargets;

  bool buildFile();
  bool buildConfigurations();
  bool buildConfiguration(const String& configuration);
  bool buildTargets();
};
