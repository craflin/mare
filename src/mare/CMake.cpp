
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cstring>

#include "Engine.h"

#include "Tools/Assert.h"
#include "Tools/Error.h"
#include "Tools/Word.h"
#include "Tools/Directory.h"

#include "CMake.h"

bool CMake::generate(const Map<String, String>& userArgs)
{
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
    Map<String, String> cApplication;
    cApplication.append("command", "__Application");
    cApplication.append("outputs", "$(buildDir)/$(target)$(target)$(if $(Win32),.exe)");
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

  //
  if(!readFile())
    return false;

  // generate solution file
  if(!generateWorkspace())
    return false;

  return true;
}

bool CMake::readFile()
{
  // get some global keys
  engine.enterRootKey();
  workspaceName = engine.getFirstKey("name");
  List<String> allPlatforms, allConfigurations, allTargets;
  engine.getKeys("platforms", allPlatforms);
  engine.getKeys("configurations", allConfigurations);
  engine.getKeys("targets", allTargets);
  engine.leaveKey();

  // do something for each target in each configuration
  for(const List<String>::Node* i = allPlatforms.getFirst(); i; i = 0) // just use the first platform since CMake does not really support multiple target platforms
  {
    const String& platform = i->data;
    for(const List<String>::Node* i = allConfigurations.getFirst(); i; i = i->getNext())
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
        //engine.addDefaultKey(i->data, i->data);
        engine.enterRootKey();
        VERIFY(engine.enterKey("targets"));
        if(!engine.enterKey(i->data))
        {
          engine.error(String().format(256, "cannot find target \"%s\"", i->data.getData()));
          return false;
        }

        Map<String, Project>::Node* node = projects.find(i->data);
        Project& project = node ? node->data : projects.append(i->data, Project(i->data));
        //bool isNewProject = node && true;

        Project::Config& projectConfig = project.configs.append(configName, Project::Config(configName));

        /*
        if(isNewProject)
        {
          String filterName = engine.getFirstKey("folder", false);
          if(!filterName.isEmpty())
          {
            Map<String, ProjectFilter>::Node* node = projectFilters.find(filterName);
            ProjectFilter& filter = node ? node->data : projectFilters.append(filterName, ProjectFilter(createSomethingLikeGUID(filterName)));
            filter.projects.append(project);
          }
        }
        */

        //engine.getKeys("buildCommand", projectConfig.buildCommand, false);
        //engine.getKeys("reBuildCommand", projectConfig.reBuildCommand, false);
        //engine.getKeys("cleanCommand", projectConfig.cleanCommand, false);
        projectConfig.buildDir = engine.getFirstKey("buildDir", true);

        engine.getKeys("command", projectConfig.command, false);
        projectConfig.firstOutput = engine.getFirstKey("outputs", false);

        if(!projectConfig.command.isEmpty())
        {
          String firstCommand = projectConfig.command.getFirst()->data;
          if(/*firstCommand == "__Custom" ||*/ firstCommand == "__Application" || firstCommand == "__StaticLibrary" || firstCommand == "__DynamicLibrary")
          {/*
            if(firstCommand == "__Custom")
            {
              projectConfig.customBuild = true;
              firstCommand = (projectConfig.command.getSize() > 1) ? projectConfig.command.getFirst()->getNext()->data : String();
            }
            */

            if(firstCommand == "__Application")
              projectConfig.type = "Executable";
            else if(firstCommand == "__StaticLibrary")
              projectConfig.type = "Static Library";
            else if(firstCommand == "__DynamicLibrary")
              projectConfig.type = "Dynamic Library";
            projectConfig.command.clear();
          }
        }
        //if(!projectConfig.buildCommand.isEmpty())
          //projectConfig.customBuild = true;

        /*
        engine.getKeys("cppFlags", projectConfig.cppFlags, true);
        List<String> linkFlags;
        engine.getKeys("linkFlags", linkFlags, true);
        for(const List<String>::Node* i = linkFlags.getFirst(); i; i = i->getNext())
          projectConfig.linkFlags.append(i->data, 0);
        projectConfig.firstOutput = engine.getFirstKey("outputs", false);
        engine.getKeys("defines", projectConfig.defines, true);
        */
        engine.getKeys("includePaths", projectConfig.includePaths, true);
        engine.getKeys("libPaths", projectConfig.libPaths, true);
        /*
        engine.getKeys("libs", projectConfig.libs, true);
        */
        List<String> dependencies;
        engine.getKeys("dependencies", dependencies, false);
        for(const List<String>::Node* i = dependencies.getFirst(); i; i = i->getNext())
          if(!project.dependencies.find(i->data))
            project.dependencies.append(i->data);
        List<String> root;
        engine.getKeys("root", root, true);
        for(const List<String>::Node* i = root.getFirst(); i; i = i->getNext())
          project.roots.append(i->data);
        if(engine.enterKey("files"))
        {
          List<String> files;
          engine.getKeys(files);
          for(const List<String>::Node* i = files.getFirst(); i; i = i->getNext())
          {
            Map<String, Project::File>::Node* node = project.files.find(i->data);
            Project::File& file = node ? node->data : project.files.append(i->data, Project::File(i->data));
            engine.enterUnnamedKey();
            engine.addDefaultKey("file", i->data);
            VERIFY(engine.enterKey(i->data));
            file.folder = engine.getFirstKey("folder", false);
            engine.leaveKey(); // VERIFY(engine.enterKey(i->data));
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

bool CMake::generateWorkspace()
{
  // remove projects not creating any output files
  for(Map<String, Project>::Node* i = projects.getFirst(), * nexti; i; i = nexti)
  {
    nexti = i->getNext();
    if(!i->data.files.isEmpty())
      continue;
    for(const Map<String, Project::Config>::Node* j = i->data.configs.getFirst(); j; j = j->getNext())
      if(!j->data.command.isEmpty() || !j->data.firstOutput.isEmpty() || !j->data.type.isEmpty())
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

  // open output file
  fileOpen("CMakeLists.txt");

  fileWrite("cmake_minimum_required(VERSION 2.6)\n\n");
  fileWrite(String("project(") + workspaceName + ")\n\n");
  
  // write project list
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
    if(!generateProject(i->data))
      return false;

  //
  fileClose();
  return true;
}


bool CMake::generateProject(Project& project)
{
  /*

  // try generating a unified include paths list
  Map<String, void*> includeDirsSet;
  List<String> includeDirs;
  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    const Project::Config& config = i->data;
    for(const List<String>::Node* i = config.includePaths.getFirst(); i; i = i->getNext())
    {
      String path = i->data;
      if(strncmp(path.getData(), config.buildDir.getData(), config.buildDir.getLength()) == 0 && strchr("/\\", path.getData()[config.buildDir.getLength()]))
        path = String("${CMAKE_BINARY_DIR}") + path.substr(config.buildDir.getLength());
      if(!includeDirsSet.find(path))
      {
        includeDirsSet.append(path);
        includeDirs.append(path);
      }
    }
  }
  fileWrite(String("include_directories(") + Word::join(includeDirs) + ")\n");

  // try generating a unified lib paths list
  Map<String, void*> linkDirsSet;
  List<String> linkDirs;
  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    const Project::Config& config = i->data;
    for(const List<String>::Node* i = config.libPaths.getFirst(); i; i = i->getNext())
    {
      String path = i->data;
      if(strncmp(path.getData(), config.buildDir.getData(), config.buildDir.getLength()) == 0 && strchr("/\\", path.getData()[config.buildDir.getLength()]))
        path = String("${CMAKE_BINARY_DIR}") + path.substr(config.buildDir.getLength());
      if(!linkDirsSet.find(path))
      {
        linkDirsSet.append(path);
        linkDirs.append(path);
      }
    }
  }
  fileWrite(String("link_directories(") + Word::join(linkDirs) + ")\n");

  // add_definitions(-DFOO -DBAR ...)
  */
  
  /*
  add_executable (helloDemo demo.cxx demo_b.cxx)
  add_library (Hello [STATIC | SHARED ] hello.cxx) 

  target_link_libraries (helloDemo Hello) 

  set(CMAKE_CXX_FLAGS "-g -Wall") ? 
  
  */

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
