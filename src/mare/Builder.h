
#pragma once

#include "Tools/List.h"
#include "Tools/Map.h"

class Engine;
class Word;
class String;

class Builder
{
public:

  Builder(Engine& engine, List<String>& inputPlatforms, List<String>& inputConfigs, List<String>& inputTargets, bool showDebug, bool clean, bool rebuild, int jobs) :
    engine(engine), showDebug(showDebug), clean(clean), rebuild(rebuild), jobs(jobs), inputPlatforms(inputPlatforms), inputConfigs(inputConfigs), inputTargets(inputTargets) {}

  bool build(const Map<String, String>& userArgs);

  static String join(const List<String>& words);
  static String join(const List<Word>& words);

private:
  Engine& engine;
  bool showDebug;
  bool clean;
  bool rebuild;
  int jobs;

  List<String>& inputPlatforms;
  List<String>& inputConfigs;
  List<String>& inputTargets;
  List<String> allTargets;

  bool buildFile();
  bool buildTargets(const String& platform, const String& configuration);

  friend class Rule;
};
