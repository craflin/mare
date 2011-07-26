
#pragma once

#include "Tools/List.h"
#include "Tools/Map.h"
#include "Tools/String.h"

class Engine;

class Builder
{
public:

  Builder(Engine& engine, List<String>& inputPlatforms, List<String>& inputConfigs, List<String>& inputTargets, bool showDebug, bool clean, bool rebuild, int jobs) :
    engine(engine), showDebug(showDebug), clean(clean), rebuild(rebuild), jobs(jobs), inputPlatforms(inputPlatforms), inputConfigs(inputConfigs), inputTargets(inputTargets) {}

  bool build(const Map<String, String>& userArgs);

private:
  Engine& engine;
  bool showDebug;
  bool clean;
  bool rebuild;
  int jobs;

  List<String>& inputPlatforms;
  List<String>& inputConfigs;
  List<String>& inputTargets;

  bool buildFile();
  bool buildConfigurations();
  bool buildConfiguration(const String& configuration);
  bool buildTargets();
};
