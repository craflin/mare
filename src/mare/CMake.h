
#pragma once

#include "Tools/Map.h"
#include "Tools/List.h"
#include "Tools/String.h"
#include "Tools/File.h"

class Engine;

class CMake
{
public:

  CMake(Engine& engine, List<String>& inputConfigs) : engine(engine), inputConfigs(inputConfigs) {}

  bool generate(const Map<String, String>& userArgs);

private:

  class Project
  {
  public:
    class Config
    {
    public:
      String name; /**< The name of the configuration without the platform extension */

      String buildDir;
      List<String> command;
      String firstOutput;
      List<String> defines;
      List<String> includePaths;
      List<String> libPaths;
      List<String> libs;
      List<String> cppFlags;
      List<String> cFlags;
      List<String> linkFlags;
      List<String> dependencies;
      
      Config(const String& name) : name(name) {}
    };

    class File
    {
    public:
      class Config
      {
      public:
        List<String> command;
      };

      String name;
      Map<String, Config> configs;

      //
      String type; /*< combined type */

      File(const String& name) : name(name) {}
    };

    String name;
    Map<String, Config> configs;
    Map<String, File> files;

    String type; /*< combined type */
    List<String> sourceFiles;
    List<String> includePaths;
    List<String> linkPaths;
    List<String> libs;
    List<String> dependencies;

    Project(const String& name) : name(name) {}
  };

  Engine& engine;
  List<String>& inputConfigs;

  File file;

  String workspaceName;
  Map<String, void*> configs;
  Map<String, Project> projects;
  
  String openedFile; /**< The file that is currently written */

  bool readFile();

  bool processData();

  bool generateWorkspace();
  bool generateProjects();
  bool generateProject(Project& project);

  void fileOpen(const String& name);
  void fileWrite(const String& data);
  void fileClose();

  static String join(const List<String>& items);
};

