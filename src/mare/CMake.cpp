
#include <cstdlib>
#include <cstdio>

#include "Engine.h"
#include "Tools/Assert.h"
#include "Tools/Error.h"
#include "Tools/Word.h"
#include "Tools/Directory.h"

#include "CMake.h"

bool CMake::generate(const Map<String, String>& userArgs)
{
  // add default keys
  engine.addDefaultKey("tool", "CMake");
  engine.addDefaultKey("CMake", "CMake");
#if defined(_WIN32) || defined(__CYGWIN__)
  String platform("Win32");
#elif defined(__linux)
  String platform("Linux");
#elif defined(__APPLE__) && defined(__MACH__)
  String platform("MacOSX");
#else
  String platform("unknown");
  // add your os :)
  // http://predef.sourceforge.net/preos.html
#endif
  engine.addDefaultKey("host", platform); // the platform on which the compiler is run
  engine.addDefaultKey("platforms", platform); // the target platform of the compiler
  engine.addDefaultKey("configurations", "Debug Release");
  engine.addDefaultKey("targets"); // an empty target list exists per default
  engine.addDefaultKey("buildDir", "$(configuration)");
  {
    Map<String, String> cSource;
    cSource.append("command", "__Source");
    engine.addDefaultKey("cppSource", cSource);
    engine.addDefaultKey("cSource", cSource);
    engine.addDefaultKey("rcSource", cSource);
  }
  {
    Map<String, String> cApplication;
    cApplication.append("command", "__Application");
    cApplication.append("outputs", "$(buildDir)/$(target)$(if $(Win32),.exe)");
    engine.addDefaultKey("cppApplication", cApplication);
    engine.addDefaultKey("cApplication", cApplication);
  }
  {
    Map<String, String> cDynamicLibrary;
    cDynamicLibrary.append("command", "__DynamicLibrary");
    cDynamicLibrary.append("outputs", "$(buildDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.dll,.so)");
    engine.addDefaultKey("cppDynamicLibrary", cDynamicLibrary);
    engine.addDefaultKey("cDynamicLibrary", cDynamicLibrary);
  }
  {
    Map<String, String> cStaticLibrary;
    cStaticLibrary.append("command", "__StaticLibrary");
    cStaticLibrary.append("outputs", "$(buildDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.lib,.a)");
    engine.addDefaultKey("cppStaticLibrary", cStaticLibrary);
    engine.addDefaultKey("cStaticLibrary", cStaticLibrary);
  }

  // add user arguments
  for(const Map<String, String>::Node* i = userArgs.getFirst(); i; i = i->getNext())
    engine.addDefaultKey(i->key, i->data);

  // step #1: read input file
  if(!readFile())
    return false;

  // step #2 ...
  if(!processData())
    return false;

  // step #3: generate output files
  if(!generateWorkspace())
    return false;
  if(!generateProjects())
    return false;

  return true;
}

bool CMake::readFile()
{
  // get some global keys
  engine.enterRootKey();
  workspaceName = engine.getFirstKey("name");
  List<String> allPlatforms/*, allConfigurations*/, allTargets;
  engine.getKeys("platforms", allPlatforms);
  //engine.getKeys("configurations", allConfigurations);
  engine.getKeys("targets", allTargets);

// read default or check input configuration names
  VERIFY(engine.enterKey("configurations"));
  if(inputConfigs.isEmpty())
  {
    String firstConfiguration = engine.getFirstKey();
    if(!firstConfiguration.isEmpty())
      inputConfigs.append(firstConfiguration);
    else
    {
      engine.error("cannot find any configurations");
      return false;
    }
  }
  else
    for(const List<String>::Node* i = inputConfigs.getFirst(); i; i = 0 /*i->getNext()*/)
      if(!engine.hasKey(i->data))
      {
        engine.error(String().format(256, "cannot find configuration \"%s\"", i->data.getData()));
        return false;
      }
  engine.leaveKey();

  engine.leaveKey();

  // do something for each target in each configuration
  for(const List<String>::Node* i = allPlatforms.getFirst(); i; i = 0) // just use the first platform since CMake does not really support multiple target platforms
  {
    const String& platform = i->data;
    for(const List<String>::Node* i = inputConfigs.getFirst(); i; i = 0/*i->getNext()*/)
    {
      const String& configName = i->data;
      configs.append(configName);

      for(const List<String>::Node* i = allTargets.getFirst(); i; i = i->getNext())
      {
        engine.enterUnnamedKey();
        engine.addDefaultKey("platform", platform);
        engine.addDefaultKey(platform, platform);
        engine.addDefaultKey("configuration", configName);
        engine.addDefaultKey(configName, configName);
        engine.addDefaultKey("target", i->data);

        engine.enterRootKey();
        VERIFY(engine.enterKey("targets"));
        if(!engine.enterKey(i->data))
        {
          engine.error(String().format(256, "cannot find target \"%s\"", i->data.getData()));
          return false;
        }

        Map<String, Project>::Node* node = projects.find(i->data);
        Project& project = node ? node->data : projects.append(i->data, Project(i->data));
        Project::Config& projectConfig = project.configs.append(configName, Project::Config(configName));

        engine.getText("command", projectConfig.command, false);
        engine.getText("message", projectConfig.message, false);
        engine.getKeys("dependencies", projectConfig.dependencies, false);
        engine.getKeys("outputs", projectConfig.output, false);
        engine.getKeys("inputs", projectConfig.input, false);

        projectConfig.buildDir = engine.getFirstKey("buildDir", true);
        engine.getKeys("cppFlags", projectConfig.cppFlags, true);
        engine.getKeys("cFlags", projectConfig.cFlags, true);
        engine.getKeys("linkFlags", projectConfig.linkFlags, true);
        engine.getKeys("defines", projectConfig.defines, true);
        engine.getKeys("includePaths", projectConfig.includePaths, true);
        engine.getKeys("libPaths", projectConfig.libPaths, true);
        engine.getKeys("libs", projectConfig.libs, true);

        if(engine.enterKey("files"))
        {
          List<String> files;
          engine.getKeys(files);
          for(const List<String>::Node* i = files.getFirst(); i; i = i->getNext())
          {
            Map<String, Project::File>::Node* node = project.files.find(i->data);
            Project::File& file = node ? node->data : project.files.append(i->data, Project::File(i->data));
            Project::File::Config& fileConfig = file.configs.append(configName);

            engine.enterUnnamedKey();
            engine.addDefaultKey("file", i->data);
            VERIFY(engine.enterKey(i->data));

            engine.getText("command", fileConfig.command, false);
            engine.getText("message", fileConfig.message, false);
            engine.getKeys("inputs", fileConfig.input, false);
            engine.getKeys("outputs", fileConfig.output, false);

            engine.leaveKey();
            engine.leaveKey();
          }

          engine.leaveKey();
        }

        engine.leaveKey();
        engine.leaveKey();
        engine.leaveKey();
        engine.leaveKey();
      }
    }
  }

  return true;
}

bool CMake::processData()
{
  // remove projects not creating any output files
  for(Map<String, Project>::Node* i = projects.getFirst(), * nexti; i; i = nexti)
  {
    nexti = i->getNext();
    if(!i->data.files.isEmpty())
      continue;
    for(const Map<String, Project::Config>::Node* j = i->data.configs.getFirst(); j; j = j->getNext())
      if(!j->data.command.isEmpty() || !j->data.output.isEmpty() || !i->data.type.isEmpty())
        goto next;
    projects.remove(i);
  next:;
  }

  // avoid creating an empty and possibly nameless solution file
  if(projects.isEmpty())
  {
    engine.error("cannot find any targets");
    return false;
  }

  // create solution file name
  if(workspaceName.isEmpty() && !projects.isEmpty())
    workspaceName = projects.getFirst()->data.name;

  // prepare some data for each project and each file
  Map<String, void*> sourceFilesSet;
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    Project& project = i->data;

    // determine project type
    for(Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
    {
      Project::Config& config = i->data;
      for(List<String>::Node* i = config.output.getFirst(); i; i = i->getNext())
        i->data = relative2absolute(i->data);
      for(List<String>::Node* i = config.input.getFirst(); i; i = i->getNext())
        i->data = relative2absolute(i->data);

      String type = config.command.isEmpty() ? String() : Word::first(config.command.getFirst()->data);
      if(type != "__Application" && type != "__DynamicLibrary" && type != "__StaticLibrary")
        type = "__Command";
      if(!project.type.isEmpty() && project.type != type)
      {
        // TODO: error message
        return false;
      }
      project.type = type;
    }

    // determine dependencies
    Map<String, void*> dependencySet;
    for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
    {
      const Project::Config& config = i->data;
      for(const List<String>::Node* i = config.dependencies.getFirst(); i; i = i->getNext())
        if(!dependencySet.find(i->data))
        {
          dependencySet.append(i->data);
          project.dependencies.append(i->data);
        }
    }
    // TODO: add error message when dependencies are inconsistent among configurations?
    // TODO: add dependencies from source files?

    // determine file types
    for(Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
    {
      Project::File& file = i->data;

      for(Map<String, Project::File::Config>::Node* i = file.configs.getFirst(); i; i = i->getNext())
      {
        Project::File::Config& fileConfig = i->data;
        String type = fileConfig.command.isEmpty() ? String() : Word::first(fileConfig.command.getFirst()->data);
        if(type != "__Source")
          if(!fileConfig.command.isEmpty() && !fileConfig.output.isEmpty())
            type = "__Command";
        if(!file.type.isEmpty() && file.type != type)
        {
          // TODO: error message
          return false;
        }
        file.type = type;
        for(List<String>::Node* i = fileConfig.input.getFirst(); i; i = i->getNext())
          i->data = relative2absolute(i->data);
        for(List<String>::Node* i = fileConfig.output.getFirst(); i; i = i->getNext())
        {
          if(type == "__Command")
          {
            if(!sourceFilesSet.find(i->data))
            {
              sourceFilesSet.append(i->data);
              project.sourceFiles.append(relative2absolute(i->data));
            }
          }
          i->data = relative2absolute(i->data);
        }
      }

      if(file.type == "__Source")
      {
        if(!sourceFilesSet.find(file.name))
        {
          sourceFilesSet.find(file.name);
          project.sourceFiles.append(relative2absolute(file.name));
        }
      }
    }

    // create combined include path list
    Map<String, void*> includePathSet;
    for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
    {
      const Project::Config& config = i->data;
      for(const List<String>::Node* i = config.includePaths.getFirst(); i; i = i->getNext())
        if(!includePathSet.find(i->data))
        {
          includePathSet.append(i->data);
          project.includePaths.append(relative2absolute(i->data));
        }
    }
    // TODO: add error message when include paths are inconsistent among configurations

    // create combined link path list
    Map<String, void*> linkPathSet;
    for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
    {
      const Project::Config& config = i->data;
      for(const List<String>::Node* i = config.libPaths.getFirst(); i; i = i->getNext())
        if(!linkPathSet.find(i->data))
        {
          linkPathSet.append(i->data);
          project.linkPaths.append(relative2absolute(i->data));
        }
    }
    // TODO: remove output directories of other targets?
    // TODO: add error message when link paths are inconsistent among configurations
  
    // create combined lib list
    Map<String, void*> libSet;
    for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
    {
      const Project::Config& config = i->data;
      for(const List<String>::Node* i = config.libs.getFirst(); i; i = i->getNext())
        if(!libSet.find(i->data))
        {
          libSet.append(i->data);
          project.libs.append(i->data);
        }
    }
    // TODO: add error message when libs are inconsistent among configurations
  }

  
  return true;
}

bool CMake::generateWorkspace()
{
  // open output file
  fileOpen("CMakeLists.txt");

  fileWrite("cmake_minimum_required(VERSION 2.8)\n\n");
  fileWrite(String("project(") + workspaceName + ")\n\n");
  /*
  // set configurations
  fileWrite("if(CMAKE_CONFIGURATION_TYPES)\n");
  List<String> configurations;
  for(const Map<String, void*>::Node* i = configs.getFirst(); i; i = i->getNext())
    configurations.append(i->key);
  fileWrite(String("  set(CMAKE_CONFIGURATION_TYPES ") + join(configurations) + ")\n");
  fileWrite("  set(CMAKE_CONFIGURATION_TYPES \"${CMAKE_CONFIGURATION_TYPES}\" CACHE STRING\n");
  fileWrite("    \"Supported configuration types\"\n");
  fileWrite("     FORCE)\n");
  fileWrite("endif()\n\n");
  */
  // write project list
  for(const Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
    fileWrite(String("add_subdirectory(") + i->key + ")\n");

  fileClose();
  return true;
}

bool CMake::generateProjects()
{
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
    if(!generateProject(i->data))
      return false;
  return true;
}

bool CMake::generateProject(Project& project)
{
  Directory::create(project.name);
  fileOpen(project.name + "/CMakeLists.txt");

  // fileWrite("cmake_policy(SET CMP0015 NEW)\n");

  if(!project.includePaths.isEmpty())
    fileWrite(String("include_directories(") + join(project.includePaths) + ")\n");

  if(!project.linkPaths.isEmpty())
    fileWrite(String("link_directories(") + join(project.linkPaths) + ")\n");

  for(const Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
  {
    const Project::File& file = i->data;
    for(const Map<String, Project::File::Config>::Node* i = file.configs.getFirst(); i; i = 0/*i->getNext()*/)
    {
      const Project::File::Config& fileConfig = i->data;
      if(file.type != "__Command")
        continue;
      fileWrite("add_custom_command(\n");
      fileWrite(String("  OUTPUT ") + join(fileConfig.output) + "\n");
      for(const List<String>::Node* i = fileConfig.command.getFirst(); i; i = i->getNext())
        fileWrite(String("  COMMAND ") + i->data + "\n");
      fileWrite(String("  DEPENDS ") + join(fileConfig.input) + "\n");
      fileWrite("  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..\n");
      if(!fileConfig.message.isEmpty())
        fileWrite(String("  COMMENT \"") + fileConfig.message.getFirst()->data + "\"\n");
      fileWrite("  )\n");
    }
  }

  if(project.type == "__Application")
    fileWrite(String("add_executable(") + project.name + " " + join(project.sourceFiles) + ")\n");
  else if(project.type == "__DynamicLibrary")
    fileWrite(String("add_library(") + project.name + " SHARED " + join(project.sourceFiles) + ")\n");
  else if(project.type == "__StaticLibrary")
    fileWrite(String("add_library(") + project.name + " STATIC " + join(project.sourceFiles) + ")\n");
  else if(project.type == "__Command")
  {
    for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = 0/*i->getNext()*/)
    {
      const Project::Config& config = i->data;
      if(!config.output.isEmpty() && !config.command.isEmpty())
      {
        fileWrite("add_custom_command(\n");
        fileWrite(String("  OUTPUT ") + join(config.output) + "\n");
        for(const List<String>::Node* i = config.command.getFirst(); i; i = i->getNext())
          fileWrite(String("  COMMAND ") + i->data + "\n");
        fileWrite(String("  DEPENDS ") + join(config.input) + "\n");
        fileWrite("  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..\n");
        if(!config.message.isEmpty())
          fileWrite(String("  COMMENT \"") + config.message.getFirst()->data + "\"\n");
        fileWrite("  )\n");
      }
      fileWrite(String("add_custom_target(") + project.name + " ALL\n");
      fileWrite(String("  DEPENDS ") + join(project.sourceFiles) + "\n");
      fileWrite("  )\n");
    }
  }
  
  if(!project.libs.isEmpty())
    fileWrite(String("target_link_libraries(") + project.name + " " + join(project.libs) + ")\n");
  
  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = 0/*i->getNext()*/)
  {
    const Project::Config& config = i->data;
    //String configUpName = i->key;
    //configUpName.uppercase();

    if(!config.output.isEmpty())
    {
      String outputDirectory = File::getDirname(config.output.getFirst()->data);
      fileWrite(String("set_property(TARGET ") + project.name + " PROPERTY ARCHIVE_OUTPUT_DIRECTORY " + outputDirectory + ")\n");
      fileWrite(String("set_property(TARGET ") + project.name + " PROPERTY LIBRARY_OUTPUT_DIRECTORY " + outputDirectory + ")\n");
      fileWrite(String("set_property(TARGET ") + project.name + " PROPERTY RUNTIME_OUTPUT_DIRECTORY " + outputDirectory + ")\n");

      String outputName = File::getWithoutExtension(File::getBasename(config.output.getFirst()->data));
      outputName.patsubst("lib%", "%");
      fileWrite(String("set_property(TARGET ") + project.name + " PROPERTY OUTPUT_NAME " + outputName + ")\n");
    }

    if(!config.linkFlags.isEmpty())
    {
      if(project.type == "__StaticLibrary")
        {}
        // fileWrite(String("set_property(TARGET ") + project.name + " PROPERTY STATIC_LIBRARY_FLAGS " + join(config.linkFlags) + ")\n");
      else
        fileWrite(String("set_property(TARGET ") + project.name + " PROPERTY LINK_FLAGS " + join(config.linkFlags) + ")\n");
    }

    if(!config.defines.isEmpty())
      fileWrite(String("set_property(TARGET ") + project.name + " PROPERTY COMPILE_DEFINITIONS " + join(config.defines) + ")\n");

    if(!config.cppFlags.isEmpty())
      fileWrite(String("set(CMAKE_CXX_FLAGS \"") + join(config.cppFlags) + "\")\n");
    if(!config.cFlags.isEmpty())
      fileWrite(String("set(CMAKE_C_FLAGS \"") + join(config.cFlags) + "\")\n");
  }

  if(!project.dependencies.isEmpty())
    fileWrite(String("add_dependencies(") + project.name + " " + join(project.dependencies) + ")\n");

  fileClose();
  return true;
}

void CMake::fileOpen(const String& name)
{
  if(!file.open(name, File::writeFlag))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
  openedFile = name;
}

void CMake::fileWrite(const String& data)
{
  if(!file.write(data))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
}

void CMake::fileClose()
{
  file.close();
  if(!openedFile.isEmpty())
  {
    puts(openedFile.getData());
    fflush(stdout);
  }
  openedFile.clear();
}

String CMake::join(const List<String>& items)
{
  String result;
  const List<String>::Node* i = items.getFirst();
  if(i)
  {
    result = i->data;
    for(i = i->getNext(); i; i = i->getNext())
    {
      result.append(' ');
      result.append(i->data);
    }
  }
  return result;
}

String CMake::relative2absolute(const String& path)
{
  if(path.getData()[0] == '/')
    return path;
  return String("${CMAKE_CURRENT_SOURCE_DIR}/../") + path;  
}

