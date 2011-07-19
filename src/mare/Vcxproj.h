
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

  class Config
  {
  public:
    String name;
    String platform;

    Config(const String& name, const String& platform) : name(name), platform(platform) {}
  };

  class Project
  {
  public:
    class Config
    {
    public:
      String name; /**< The name of the configuration without the platform extension */
      String platform;

      List<String> buildCommand; /**< For Makefile projects */
      List<String> reBuildCommand; /**< For Makefile projects */
      List<String> cleanCommand; /**< For Makefile projects */
      List<String> preBuildCommand;
      List<String> preLinkCommand;
      List<String> postBuildCommand;
      String buildDir;
      String firstCommand;
      String firstOutput;
      List<String> outputs;
      List<String> cppFlags;
      Map<String, void*> linkFlags;
      List<String> defines;
      List<String> includePaths;
      List<String> libPaths;
      List<String> libs;

      Config(const String& name, const String& platform) : name(name), platform(platform) {}
    };

    class File
    {
    public:
      class Config
      {
      public:
        List<String> message;
        List<String> command;
        List<String> outputs;
        List<String> inputs;
      };

      String type;
      Map<String, Config> configs;
    };

    String name;
    String guid;
    Map<String, Config> configs;
    Map<String, File> files;
    Map<String, void*> dependencies;

    Project(const String& name, const String& guid) : name(name), guid(guid) {}
  };

  class ProjectFilter
  {
  public:
    String guid;
    List<Project*> projects;

    ProjectFilter(const String& guid) : guid(guid) {}
  };

  class Option
  {
  public:
    String name;
    String value;

    Option() {}

    Option(const String& name, const String& value) : name(name), value(value) {}
  };

  Engine& engine;
  int version; /**< Version of the vcxproj file format e.g. 2010 */

  File file;

  String solutionName;
  Map<String, Config> configs;
  Map<String, void*> activesProjects;
  Map<String, Project> projects;
  Map<String, ProjectFilter> projectFilters;
  Map<String, Option> knownCppOptions;
  Map<String, Option> knownLinkOptions;

  bool generateSln();
  bool generateVcxprojs();
  bool generateVcxproj(Project& project);
  bool generateVcxprojFilter(Project& project);

  void fileOpen(const String& name);
  void fileWrite(const String& data);
  void fileClose() {file.close();}

  String createSomethingLikeGUID(const String& name);
  String join(const List<String>& items, char sep = ';', const String& suffix = String()) const;
  String joinCommands(const List<String>& commands) const;
};

