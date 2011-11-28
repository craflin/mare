
#pragma once

#include "Tools/Map.h"
#include "Tools/List.h"
#include "Tools/String.h"
#include "Tools/File.h"

class Engine;

class Make
{
public:

  Make(Engine& engine) : engine(engine) {}

  bool generate(const Map<String, String>& userArgs);

private:

  class Platform
  {
  public:
    class Config
    {
    public:
      class Target
      {
      public:
        class File
        {
        public:

          String name;
          String type;
          List<String> output;
          List<String> input;
          List<String> command;
          List<String> message;
          List<String> dependencies;

          File(const String& name) : name(name) {}
        };

        Target(const String& name) : name(name) {}

        String name;
        String type;
        List<File> files;
        List<String> output;
        List<String> input;
        List<String> command;
        List<String> message;
        List<String> dependencies;
        String buildDir;

        Map<String, void*> outputDirs;
        List<String> objects;
      };

      Config(const String& name) : name(name) {}

      String name;
      List<Target> targets;
    };

    Platform(const String& name) : name(name) {}

    String name;
    List<Config> configs;
  };

  Engine& engine;

  File file;
  String openedFile; /**< The file that is currently written */

  List<Platform> platforms;

  bool readFile();

  bool processData();

  bool generateMakefile();
  void generateMakefilePlatform(Platform& platform);
  void generateMakefileConfig(const Platform& platform, const Platform::Config& config);
  bool generateTargetMakefile(const Platform& platform, const Platform::Config& config, const Platform::Config::Target& target);

  void fileOpen(const String& name);
  void fileWrite(const String& data);
  void fileClose();

  static String join(const List<String>& items);
  static String joinCommands(const String& prefix, const String& suffix, const List<String>& commands);
};

