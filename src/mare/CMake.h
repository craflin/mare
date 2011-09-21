
#pragma once

#include "Tools/Map.h"
#include "Tools/List.h"
#include "Tools/String.h"
#include "Tools/File.h"

class Engine;

class CMake
{
public:

  CMake(Engine& engine) : engine(engine) {}

  bool generate(const Map<String, String>& userArgs);

private:

  class Project
  {
  public:
    class Config
    {
    public:
      String name; /**< The name of the configuration without the platform extension */

      //List<String> buildCommand; /**< For CustomBuild projects */
      //List<String> reBuildCommand; /**< For CustomBuild projects */
      //List<String> cleanCommand; /**< For CustomBuild projects */
      /*
      List<String> preBuildCommand;
      List<String> preLinkCommand;
      List<String> postBuildCommand;
      */
      String buildDir;
      String type;
      List<String> command;
      //List<String> message;
      String firstOutput;
      //bool customBuild;
      /*
      List<String> outputs;
      List<String> inputs;
      List<String> dependencies;
      List<String> cppFlags;
      Map<String, void*> linkFlags;
      List<String> defines;
      */
      List<String> includePaths;
      List<String> libPaths;
      /*
      List<String> libs;
      */
      Config(const String& name) : name(name)/*, customBuild(false)*/ {}
    };

    class File
    {
    public:
      String name;
      String folder;

      File(const String& name) : name(name) {}
    };

    String name;
    Map<String, Config> configs;
    Map<String, File> files;
    Map<String, void*> dependencies;
    Map<String, void*> roots;

    Project(const String& name) : name(name) {}
  };
/*
  class ProjectFilter
  {
  public:
    String guid;
    List<Project*> projects;

    ProjectFilter(const String& guid) : guid(guid) {}
  };
*/
  Engine& engine;

  File file;

  String workspaceName;
  Map<String, void*> configs;
  Map<String, Project> projects;
  //Map<String, ProjectFilter> projectFilters;

  String openedFile; /**< The file that is currently written */

  bool readFile();

  bool generateWorkspace();
  bool generateProject(Project& project);

  void fileOpen(const String& name);
  void fileWrite(const String& data);
  void fileClose();
};

