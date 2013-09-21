
#pragma once

#include "Tools/Map.h"
#include "Tools/String.h"
#include "Tools/File.h"

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
      String type;
      List<String> command;
      List<String> message;
      String firstOutput;
      List<String> outputs;
      List<String> inputs;
      List<String> dependencies;
      Map<String, void*> cAndCppFlags;
      Map<String, String> cppOptions;
      Map<String, void*> linkFlags;
      Map<String, String> linkOptions;
      Map<String, String> vsOptions;
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
        List<String> dependencies;
        Map<String, void*> cAndCppFlags;
        Map<String, String> cppOptions;
      };

      String type;
      String filter;
      Map<String, Config> configs;
      bool useDefaultSettings;

      File() : useDefaultSettings(true) {}
    };

    String name;
    String displayName;
    String guid;
    String filter;
    Map<String, Config> configs;
    Map<String, File> files;
    Map<String, void*> dependencies;
    Map<String, void*> roots;

    Project(const String& name, const String& guid) : name(name), guid(guid) {}
  };

  class ProjectFilter
  {
  public:
    String guid;
    List<Project*> projects;
    List<ProjectFilter*> filters;

    ProjectFilter(const String& guid) : guid(guid) {}
  };

  class Option
  {
  public:
    String name;
    String value;
    String unsetValue;
    String paramName;

    Option() {}

    Option(const String& name, const String& value, const String& unsetValue = String(), const String& paramName = String()) : name(name), value(value), unsetValue(unsetValue), paramName(paramName) {}

    bool hasParam(const String& option) const;
    static String getParam(const String& option);
  };

  class OptionMap : public Map<String, Option>
  {
  public:
    Node* find(const String& key);
  };

  Engine& engine;
  int version; /**< Version of the vcxproj file format e.g. 2010 */

  File file;

  String solutionName;
  Map<String, Config> configs;
  Map<String, Project> projects;
  Map<String, ProjectFilter> projectFilters;
  OptionMap knownCppOptions;
  Map<String, Option> knownLinkOptions;
  OptionMap knownVsOptions;

  String openedFile; /**< The file that is currently written */

  bool readFile();

  bool processData();
  bool resolveDependencies();

  bool generateSln();
  bool generateVcxprojs();
  bool generateVcxproj(Project& project);
  bool generateVcxprojFilter(Project& project);

  void fileOpen(const String& name);
  void fileWrite(const String& data);
  void fileClose();

  static String createSomethingLikeGUID(const String& name);
  static String join(const List<String>& items, char sep = ';', const String& suffix = String());
  static String joinCommands(const List<String>& commands);
  static String xmlEscape(const String& text);
};

