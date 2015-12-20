
#pragma once

#include "Generator.h"

class CMake : public Generator
{
public:
  CMake(Engine& engine) : Generator(engine) {}

private:
  class Library
  {
  public:
    enum Type
    {
      localType,
      externalType,
    };

  public:
    String name;
    Type type;
  };

  class ProjectConfiguration
  {
  public:
    enum Type
    {
      applicationType,
      dynamicLibraryType,
      staticLibraryType,
      customTargetType,
    };

  public:
    const Target* target;
    Type type;
    List<String> sourceFiles;
    List<const File*> customBuildFiles;
    List<Library> libs;
  };

  class Project
  {
  public:
    Map<String, ProjectConfiguration> configurations;
  };

private:
  bool writeWorkspace();
  bool writeProjects();
  bool writeProject(const String& targetName, Project& project);

private:
  static String join(const List<String>& items);
  static String joinPaths(const List<String>& items, bool absolute = false);
  static String translatePath(const String& path, bool absolute = true);

private: // Generator
  virtual void addDefaultKeys(Engine& engine);
  virtual bool processData(const Data& data);
  virtual bool writeFiles();

private:
  String workspaceName;
  List<String> configurations;
  Map<String, Project> projects;
};
