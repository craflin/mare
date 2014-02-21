
#pragma once

#include "Tools/Map.h"
#include "Tools/List.h"
#include "Tools/String.h"
#include "Tools/File.h"

class Engine;

class Generator
{
public:
  Generator(Engine& engine) : engine(engine) {}

  bool generate(const Map<String, String>& userArgs);

protected:
  class File
  {
  public:
    String folder;
    List<String> command;
    List<String> message;
    List<String> output;
    List<String> input;
    List<String> dependencies;
    bool hasCppFlags;
    List<String> cppFlags;
    bool hasCFlags;
    List<String> cFlags;
  };

  class Target
  {
  public:
    String folder;
    List<String> buildCommand;
    List<String> reBuildCommand;
    List<String> cleanCommand;
    List<String> preBuildCommand;
    List<String> preLinkCommand;
    List<String> postBuildCommand;
    String buildDir;
    List<String> command;
    List<String> message;
    List<String> output;
    List<String> input;
    List<String> dependencies;
    List<String> cppFlags;
    List<String> cFlags;
    List<String> linkFlags;
    List<String> defines;
    List<String> includePaths;
    List<String> libPaths;
    List<String> libs;
    List<String> root;
    Map<String, File> files;
  };

  class Configuration
  {
  public:
    Map<String, Target> targets;
  };
  
  class Platform
  {
  public:
    Map<String, Configuration> configurations;
  };
  
  class Data
  {
  public:
    String name; /**< Name of the workspace or solution. */
    Map<String, Platform> platforms;
  };

  virtual void addDefaultKeys(Engine& engine) = 0;
  virtual bool processData(const Data& data) = 0;
  virtual bool writeFiles() = 0;

  void fileOpen(const String& name);
  void fileWrite(const String& data);
  void fileClose();

  void error(const String& message);

private:
  Engine& engine;

  ::File file;
  String openedFile; /**< The file that is currently written */

  bool readFile(Data& data);
};
