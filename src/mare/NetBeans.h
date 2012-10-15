
#pragma once

#include "Tools/Map.h"
#include "Tools/List.h"
#include "Tools/String.h"
#include "Tools/File.h"

class Engine;

class NetBeans
{
public:

  NetBeans(Engine& engine) : engine(engine) {}

  bool generate(const Map<String, String>& userArgs);

private:

  class Project
  {
  public:
    class Config
    {
    public:
      String name; /**< The name of the configuration without the platform extension */

      List<String> buildCommand; /**< For CustomBuild projects */
      List<String> reBuildCommand; /**< For CustomBuild projects */
      List<String> cleanCommand; /**< For CustomBuild projects */
      String buildDir;
      String firstOutput;
      List<String> defines;
      List<String> includePaths;
      Config(const String& name) : name(name) {}
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
    Map<String, void*> roots;
    Map<String, void*> dependencies;

    Project(const String& name) : name(name) {}
  };

  Engine& engine;

  File file;

  Map<String, void*> configs;
  Map<String, Project> projects;

  String openedFile; /**< The file that is currently written */

  bool readFile();

  bool processData();
  bool generateProject(Project& project);

  void fileOpen(const String& name);
  void fileWrite(const String& data);
  void fileClose();

  //static String join(const List<String>& items, char sep = ';', const String& suffix = String());

  static String joinCommands(const List<String>& commands);
  static String xmlEscape(const String& text);
};

