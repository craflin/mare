
#pragma once

#include "Tools/Map.h"
#include "Tools/String.h"
#include "Tools/File.h"

class Engine;

class Vcproj
{
public:

  Vcproj(Engine& engine, int version) : engine(engine), version(version) {}

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

      String type;
      enum Language
      {
        CPP,
        C,
      } language;

      List<String> buildCommand; /**< For Makefile projects */
      List<String> reBuildCommand; /**< For Makefile projects */
      List<String> cleanCommand; /**< For Makefile projects */
      List<String> preBuildCommand;
      List<String> preLinkCommand;
      List<String> postBuildCommand;
      String buildDir;
      List<String> command;
      List<String> message;
      String firstOutput;
      List<String> outputs;
      List<String> inputs;
      List<String> dependencies;
      List<String> cppFlags;
      List<String> cFlags;
      Map<String, void*> compilerFlags;
      Map<String, void*> linkFlags;
      List<String> defines;
      List<String> includePaths;
      List<String> libPaths;
      List<String> libs;

      Map<String, String> cppOptions;
      Map<String, String> linkOptions;
      Map<String, String> librarianOptions;
      Map<String, String> vsOptions;

      Config(const String& name, const String& platform) : name(name), platform(platform), language(CPP) {}
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
        bool hasCppFlags;
        List<String> cppFlags;
        bool hasCFlags;
        List<String> cFlags;

        Map<String, String> cppOptions;
      };

      String type;
      String path;
      String filter;
      Map<String, Config> configs;
      bool useProjectCompilerFlags;

      File() : useProjectCompilerFlags(true) {}
    };

    String name;
    String displayName;
    String guid;
    String filter;
    Map<String, Config> configs;
    Map<String, File> files;
    Map<String, void*> dependencies;
    List<String> root;

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

  class OptionGroup
  {
  public:
    String name;
    String unsetValue;
    String paramName;

    OptionGroup(const String& name, const String& unsetValue = String(), const String& paramName = String()) : name(name), unsetValue(unsetValue), paramName(paramName) {}
  };

  class Option
  {
  public:
    OptionGroup* group;
    String value;

    Option() : group(0) {}

    Option(OptionGroup* group, const String& value) : group(group), value(value) {}

    bool hasParam(const String& option) const;
    static String getParamValue(const String& option);
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
  List<String> platforms;
  Map<String, Config> configs;
  Map<String, Project> projects;
  Map<String, ProjectFilter> projectFilters;

  List<OptionGroup> knownOptionGroups;
  OptionMap knownCppOptions;
  Map<String, Option> knownLinkOptions;
  OptionMap knownVsOptions;

  String openedFile; /**< The file that is currently written */

  bool readFile();

  bool processData();
  bool resolveDependencies();

  bool generateSln();
  bool generateVcprojs();
  bool generateVcproj(const Project& project);
  bool generateVcprojFiles(const Project& project);

  void fileOpen(const String& name);
  void fileWrite(const String& data);
  void fileClose();

  static String createSomethingLikeGUID(const String& name);
  static String join(const List<String>& items, char sep = ';', const String& suffix = String());
  static String joinCommands(const List<String>& commands);
  static String xmlEscape(const String& text);
};

