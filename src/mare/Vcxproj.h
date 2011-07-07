
#pragma once

#include "../libmare/Tools/Map.h"
#include "../libmare/Tools/String.h"
#include "../libmare/Tools/File.h"

class Engine;

class Vcxproj
{
public:

  Vcxproj(Engine& engine, int version) : engine(engine), version(version) {}

  bool generate(const Map<String, String>& userArgs);

private:

  class ProjectFilter
  {
  public:
    String guid;

    ProjectFilter(const String& guid) : guid(guid) {}
  };

  class Project
  {
  public:
    String name;
    String guid;
    Map<String, void*> configurations;
    ProjectFilter* filter;

    Project(const String& Name, const String& guid) : name(name), guid(guid), filter(0) {}
  };

  Engine& engine;
  int version; /**< Version of the vcxproj file format e.g. 2010 */

  File file;

  String solutionName;
  List<String> platforms;
  Map<String, void*> activesProjects;
  List<String> configurations;
  Map<String, Project> projects;
  Map<String, ProjectFilter> projectFilters;

  bool generateSln();
  void fileOpen(const String& name);
  void fileWrite(const String& data);
  String createSomethingLikeGUID(const String& name);
};

