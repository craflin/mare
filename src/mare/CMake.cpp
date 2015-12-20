
#include "Engine.h"
#include "Tools/Word.h"
#include "Tools/Directory.h"

#include "CMake.h"

void CMake::addDefaultKeys(Engine& engine)
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
  engine.addDefaultKey("outputDir", "$(buildDir)");
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
    cApplication.append("output", "$(outputDir)/$(target)$(if $(Win32),.exe)");
    engine.addDefaultKey("cppApplication", cApplication);
    engine.addDefaultKey("cApplication", cApplication);
  }
  {
    Map<String, String> cDynamicLibrary;
    cDynamicLibrary.append("command", "__DynamicLibrary");
    cDynamicLibrary.append("output", "$(outputDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.dll,.so)");
    engine.addDefaultKey("cppDynamicLibrary", cDynamicLibrary);
    engine.addDefaultKey("cDynamicLibrary", cDynamicLibrary);
  }
  {
    Map<String, String> cStaticLibrary;
    cStaticLibrary.append("command", "__StaticLibrary");
    cStaticLibrary.append("output", "$(outputDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.lib,.a)");
    engine.addDefaultKey("cppStaticLibrary", cStaticLibrary);
    engine.addDefaultKey("cStaticLibrary", cStaticLibrary);
  }
}

bool CMake::processData(const Data& data)
{
  // get configurations
  for(const Map<String, Platform>::Node* i = data.platforms.getFirst(); i; i = i->getNext())
  {
    const String& platformName = i->key;
    const Platform& platform = i->data;
    for(const Map<String, Configuration>::Node* i = platform.configurations.getFirst(); i; i = i->getNext())
    {
      String configurationName = i->key;
      if(data.platforms.getSize() > 1)
        configurationName += String("_") + platformName;
      configurations.append(configurationName);
    }
  }

  // get projects
  for(const Map<String, Platform>::Node* i = data.platforms.getFirst(); i; i = i->getNext())
  {
    const String& platformName = i->key;
    const Platform& platform = i->data;
    for(const Map<String, Configuration>::Node* i = platform.configurations.getFirst(); i; i = i->getNext())
    {
      String configurationName = i->key;
      if(data.platforms.getSize() > 1)
        configurationName += String("_") + platformName;
      const Configuration& configuration = i->data;
      for(const Map<String, Target>::Node* i = configuration.targets.getFirst(); i; i = i->getNext())
      {
        const String& targetName = i->key;
        const Target& target = i->data;
        Map<String, Project>::Node* node = projects.find(targetName);
        Project& project = node ? node->data : projects.append(targetName);
        ProjectConfiguration& projectConfig = project.configurations.append(configurationName);
        projectConfig.target = &target;
      }
    }
  }

  // remove projects not creating any output files
  for(Map<String, Project>::Node* i = projects.getFirst(), * nexti; i; i = nexti)
  {
    nexti = i->getNext();
    Project& project = i->data;
    {
      for(const Map<String, ProjectConfiguration>::Node* i = project.configurations.getFirst(); i; i = i->getNext())
      {
        const ProjectConfiguration& projectConfiguration = i->data;
        const Target& target = *projectConfiguration.target;
        if(!target.files.isEmpty())
          goto next;
        if(!target.command.isEmpty() || !target.output.isEmpty())
          goto next;
      }
    }
    projects.remove(i);
  next:;
  }

  // avoid creating an empty and possibly nameless solution file
  if(projects.isEmpty())
  {
    error("Could not find any targets.");
    return false;
  }

  // create solution file name
  if(data.name.isEmpty())
    workspaceName = projects.getFirst()->key;
  else
    workspaceName = data.name;

  // determine project and source file types
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    Project& project = i->data;
    for(Map<String, ProjectConfiguration>::Node* i = project.configurations.getFirst(); i; i = i->getNext())
    {
      ProjectConfiguration& projectConfiguration = i->data;
      const Target& target = *projectConfiguration.target;

      String type = target.command.isEmpty() ? String() : Word::first(target.command.getFirst()->data);
      if(type == "__Application")
        projectConfiguration.type = ProjectConfiguration::applicationType;
      else if(type == "__DynamicLibrary")
        projectConfiguration.type = ProjectConfiguration::dynamicLibraryType;
      else if(type == "__StaticLibrary")
        projectConfiguration.type = ProjectConfiguration::staticLibraryType;
      else
        projectConfiguration.type = ProjectConfiguration::customTargetType;

      for(const Map<String, File>::Node* i = target.files.getFirst(); i; i = i->getNext())
      {
        const File& file = i->data;

        String type = file.command.isEmpty() ? String() : Word::first(file.command.getFirst()->data);
        if(type != "__Source")
          if(!file.command.isEmpty() && !file.output.isEmpty())
            type = "__Command";
        if(type == "__Source" || projectConfiguration.type == ProjectConfiguration::customTargetType)
          projectConfiguration.sourceFiles.append(i->key);
        if(type == "__Command")
          projectConfiguration.customBuildFiles.append(&file);
      }
    }
  }

  // remove custom build output files from list of source files
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    Project& project = i->data;
    for(Map<String, ProjectConfiguration>::Node* i = project.configurations.getFirst(); i; i = i->getNext())
    {
      ProjectConfiguration& projectConfiguration = i->data;
      for(List<const File*>::Node* i = projectConfiguration.customBuildFiles.getFirst(); i; i = i->getNext())
      {
        const File& file = *i->data;
        for(const List<String>::Node* i = file.output.getFirst(); i; i = i->getNext())
        {
          const String& outputFile = i->data;
          for(List<String>::Node* i = projectConfiguration.sourceFiles.getFirst(); i; i = i->getNext())
          {
            if(i->data == outputFile)
            {
              projectConfiguration.sourceFiles.remove(i);
              break;
            }
          }
        }
      }
    }
  }

  // find local lib dependencies
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    Project& project = i->data;
    for(Map<String, ProjectConfiguration>::Node* i = project.configurations.getFirst(); i; i = i->getNext())
    {
      const String& configName = i->key;
      ProjectConfiguration& projectConfiguration = i->data;
      const Target& target = *projectConfiguration.target;

      Map<String, bool> libPaths;
      for(const List<String>::Node* i = target.libPaths.getFirst(); i; i = i->getNext())
        libPaths.append(::File::simplifyPath(i->data), true);

      for(const List<String>::Node* i = target.libs.getFirst(); i; i = i->getNext())
      {
        const String& lib = i->data;

        // find lib in outputs of other projects
        String depProject;
        for(Map<String, Project>::Node* i = projects.getFirst(); i && depProject.isEmpty(); i = i->getNext())
        {
          const String& projectName = i->key;
          Project& project = i->data;
          for(Map<String, ProjectConfiguration>::Node* i = project.configurations.getFirst(); i; i = i->getNext())
          {
            if(i->key != configName)
              continue;
            ProjectConfiguration& projectConfiguration = i->data;
            if(projectConfiguration.type != ProjectConfiguration::dynamicLibraryType && projectConfiguration.type != ProjectConfiguration::staticLibraryType)
              continue;
            const Target& target = *projectConfiguration.target;
            if(target.output.isEmpty())
              continue;

            // is output dir in search paths
            String outDir = ::File::simplifyPath(::File::getDirname(target.output.getFirst()->data));
            if(outDir != "." && !libPaths.find(outDir))
              continue;

            String libName = ::File::getWithoutExtension(::File::getBasename(target.output.getFirst()->data));
            if (libName == lib || libName == String("lib") + lib)
            {
              depProject = projectName;
              break;;
            }
          }
        }

        Library& library = projectConfiguration.libs.append();
        library.name = depProject.isEmpty() ? lib : depProject;
        library.type = depProject.isEmpty() ? Library::externalType : Library::localType;
      }
    }
  }

  return true;
}

bool CMake::writeFiles()
{
  if(!writeWorkspace())
    return false;
  if(!writeProjects())
    return false;
  return true;
}

bool CMake::writeWorkspace()
{
  // open output file
  fileOpen("CMakeLists.txt");
  fileWrite("cmake_minimum_required(VERSION 2.8.1)\n\n");

  // set configurations
  fileWrite(String("set(CMAKE_CONFIGURATION_TYPES ") + join(configurations) + ")\n");
  fileWrite("set(CMAKE_CONFIGURATION_TYPES \"${CMAKE_CONFIGURATION_TYPES}\" CACHE STRING \"Supported configuration types\" FORCE)\n\n");

  // set default configuration
  if(!configurations.isEmpty())
  {
    const String& defaultConfigName = configurations.getFirst()->data;
    fileWrite(String("if(CMAKE_BUILD_TYPE STREQUAL \"\")\n"));
    fileWrite(String("set(CMAKE_BUILD_TYPE \"") + defaultConfigName + "\")\n");
    fileWrite(String("endif()\n\n"));
  }

  // set solution name
  fileWrite(String("project(") + workspaceName + ")\n\n");

  // write project list
  for(const Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
    fileWrite(String("add_subdirectory(.CMake/") + i->key + ")\n");

  fileClose();
  return true;
}

bool CMake::writeProjects()
{
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
    if(!writeProject(i->key, i->data))
      return false;
  return true;
}

bool CMake::writeProject(const String& targetName, Project& project)
{
  Directory::create(String(".CMake/") + targetName);
  fileOpen(String(".CMake/") + targetName + "/CMakeLists.txt");

  fileWrite("cmake_policy(SET CMP0015 NEW)\n\n");

  for(const Map<String, ProjectConfiguration>::Node* i = project.configurations.getFirst(); i; i = i->getNext())
  {
    const String& configName = i->key;
    const ProjectConfiguration& config = i->data;
    const Target& target = *config.target;

    if(i == project.configurations.getFirst())
      fileWrite(String("if(CMAKE_BUILD_TYPE STREQUAL \"") + configName +"\")\n");
    else
      fileWrite(String("elseif(CMAKE_BUILD_TYPE STREQUAL \"") + configName +"\")\n");

    if(!target.includePaths.isEmpty())
      fileWrite(String("include_directories(") + joinPaths(target.includePaths) + ")\n");

    if(!target.libPaths.isEmpty())
      fileWrite(String("link_directories(") + joinPaths(target.libPaths) + ")\n");

    List<String> customBuildOutput;
    for(const List<const File*>::Node* i = config.customBuildFiles.getFirst(); i; i = i->getNext())
    {
      const File& file = *i->data;
      fileWrite("add_custom_command(\n");
      fileWrite(String("  OUTPUT ") + joinPaths(file.output, true) + "\n");

      Map<String, void*> outputDirs;
      for(const List<String>::Node* i = file.output.getFirst(); i; i = i->getNext())
      {
        String dir = ::File::getDirname(i->data);
        if(dir != "." && !outputDirs.find(dir))
          outputDirs.append(dir);
      }
      for(Map<String, void*>::Node* i = outputDirs.getFirst(); i; i = i->getNext())
        fileWrite(String("  COMMAND ${CMAKE_COMMAND} -E make_directory ") + translatePath(i->key, true) + "\n");

      for(const List<String>::Node* i = file.command.getFirst(); i; i = i->getNext())
        fileWrite(String("  COMMAND ") + i->data + "\n");
      fileWrite(String("  DEPENDS ") + joinPaths(file.input) + "\n");
      fileWrite("  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..\n");
      if(!file.message.isEmpty())
        fileWrite(String("  COMMENT \"") + file.message.getFirst()->data + "\"\n");
      fileWrite("  )\n");
      for(const List<String>::Node* i = file.output.getFirst(); i; i = i->getNext())
        customBuildOutput.append(i->data);
    }

    if(config.type == ProjectConfiguration::applicationType)
      fileWrite(String("add_executable(") + targetName + " " + joinPaths(config.sourceFiles) + " " + joinPaths(customBuildOutput, true) + ")\n");
    else if(config.type == ProjectConfiguration::dynamicLibraryType)
      fileWrite(String("add_library(") + targetName + " SHARED " + joinPaths(config.sourceFiles) + " " + joinPaths(customBuildOutput, true) + ")\n");
    else if(config.type == ProjectConfiguration::staticLibraryType)
      fileWrite(String("add_library(") + targetName + " STATIC " + joinPaths(config.sourceFiles) + " " + joinPaths(customBuildOutput, true) + ")\n");
    else if(config.type == ProjectConfiguration::customTargetType)
    {
        if(!target.output.isEmpty() && !target.command.isEmpty())
        {
          fileWrite("add_custom_command(\n");
          fileWrite(String("  OUTPUT ") + joinPaths(target.output, true) + "\n");

          Map<String, void*> outputDirs;
          for(const List<String>::Node* i = target.output.getFirst(); i; i = i->getNext())
          {
            String dir = ::File::getDirname(i->data);
            if(dir != "." && !outputDirs.find(dir))
              outputDirs.append(dir);
          }
          for(Map<String, void*>::Node* i = outputDirs.getFirst(); i; i = i->getNext())
            fileWrite(String("  COMMAND ${CMAKE_COMMAND} -E make_directory ") + translatePath(i->key, true) + "\n");

          for(const List<String>::Node* i = target.command.getFirst(); i; i = i->getNext())
            fileWrite(String("  COMMAND ") + i->data + "\n");
          fileWrite(String("  DEPENDS ") + joinPaths(target.input) + "\n");
          fileWrite("  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..\n");
          if(!target.message.isEmpty())
            fileWrite(String("  COMMENT \"") + target.message.getFirst()->data + "\"\n");
          fileWrite("  )\n");
        }
        fileWrite(String("add_custom_target(") + targetName + " ALL\n");
        fileWrite(String("  DEPENDS ") + join(config.sourceFiles) + " " + joinPaths(customBuildOutput, true) + "\n");
        fileWrite("  )\n");
    }
    /*
  if(!config.libs.isEmpty())
  {
    List<String> libs;
    for(const List<Library>::Node* i = config.libs.getFirst(); i; i = i->getNext())
    {
      const Library& lib = i->data;
      if(lib.type == Library::localType)
        libs.append(lib.name);
      else
      {
        fileWrite(String("find_library(") + lib.name + "_LIBRARY " + lib.name + " PATHS " + join(target.libPaths) + ")\n");
        libs.append(String("${") + lib.name + "_LIBRARY}");
      }
    }

    fileWrite(String("target_link_libraries(") + targetName + " " + join(libs) + ")\n");
  }
  */
    if(!config.libs.isEmpty())
    {
      List<String> libs;
      for(const List<Library>::Node* i = config.libs.getFirst(); i; i = i->getNext())
        libs.append(i->data.name);
      fileWrite(String("target_link_libraries(") + targetName + " " + join(libs) + ")\n");
    }

    if(!target.cppCompiler.isEmpty())
    {
      String cppCompiler;
      for(const List<String>::Node* i = target.cppCompiler.getFirst(); i; i = i->getNext())
      {
        const String& word = i->data;
        unsigned int sep;
        if(word.find('=', sep))
          fileWrite(String("set(ENV{") + word.substr(0, sep) +  "} \"" + word.substr(sep + 1) + "\")\n");
        else
        {
          cppCompiler = word;
          break;
        }
      }

      if(!cppCompiler.isEmpty())
      {
        if(!::File::isPathAbsolute(cppCompiler) && ::File::exists(cppCompiler))
          fileWrite(String("set(CMAKE_CXX_COMPILER \"${CMAKE_CURRENT_SOURCE_DIR}/../../") + cppCompiler + "\")\n");
        else
          fileWrite(String("set(CMAKE_CXX_COMPILER \"") + cppCompiler + "\")\n");
      }
    }
    if(!target.cCompiler.isEmpty())
    {
      String cCompiler;
      for(const List<String>::Node* i = target.cCompiler.getFirst(); i; i = i->getNext())
      {
        const String& word = i->data;
        unsigned int sep;
        if(word.find('=', sep))
          fileWrite(String("set(ENV{") + word.substr(0, sep) +  "} \"" + word.substr(sep + 1) + "\")\n");
        else
        {
          cCompiler = word;
          break;
        }
      }

      if(!cCompiler.isEmpty())
      {
        if(!::File::isPathAbsolute(cCompiler) && ::File::exists(cCompiler))
          fileWrite(String("set(CMAKE_C_COMPILER \"${CMAKE_CURRENT_SOURCE_DIR}/../../") + cCompiler + "\")\n");
        else
          fileWrite(String("set(CMAKE_C_COMPILER \"") + cCompiler + "\")\n");
      }
    }

    /*
    if(!target.cppFlags.isEmpty())
      fileWrite(String("set_property(TARGET ") + targetName + " PROPERTY COMPILE_FLAGS \"" + join(target.cppFlags) + "\")\n");
    */
    if(!target.cppFlags.isEmpty())
      fileWrite(String("set(CMAKE_CXX_FLAGS \"") + join(target.cppFlags) + "\")\n");
    if(!target.cFlags.isEmpty())
      fileWrite(String("set(CMAKE_C_FLAGS \"") + join(target.cFlags) + "\")\n");

    if(!target.output.isEmpty())
    {
      String outputDirectory = ::File::getDirname(target.output.getFirst()->data);
      // Hi cmake devs! I do not know whats considered to be an ARCHIVE, LIBRARY or RUNTIME. So I will set all properties to be sure.
      fileWrite(String("set_property(TARGET ") + targetName + " PROPERTY ARCHIVE_OUTPUT_DIRECTORY \"" + translatePath(outputDirectory) + "\")\n");
      fileWrite(String("set_property(TARGET ") + targetName + " PROPERTY LIBRARY_OUTPUT_DIRECTORY \"" + translatePath(outputDirectory) + "\")\n");
      fileWrite(String("set_property(TARGET ") + targetName + " PROPERTY RUNTIME_OUTPUT_DIRECTORY \"" + translatePath(outputDirectory) + "\")\n");

      String outputName = ::File::getWithoutExtension(::File::getBasename(target.output.getFirst()->data));
      outputName.patsubst("lib%", "%");
      fileWrite(String("set_property(TARGET ") + targetName + " PROPERTY OUTPUT_NAME \"" + outputName + "\")\n");
    }

    if(!target.linkFlags.isEmpty())
    {
      if(config.type == ProjectConfiguration::staticLibraryType)
        fileWrite(String("set_property(TARGET ") + targetName + " PROPERTY STATIC_LIBRARY_FLAGS " + join(target.linkFlags) + ")\n");
      else
        fileWrite(String("set_property(TARGET ") + targetName + " PROPERTY LINK_FLAGS \"" + join(target.linkFlags) + "\")\n");
    }

    if(!target.defines.isEmpty())
      fileWrite(String("set_property(TARGET ") + targetName + " PROPERTY COMPILE_DEFINITIONS " + join(target.defines) + ")\n");

    if(!target.dependencies.isEmpty())
      fileWrite(String("add_dependencies(") + targetName + " " + join(target.dependencies) + ")\n");
  }
  fileWrite("endif()\n");

  fileClose();
  return true;
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

String CMake::joinPaths(const List<String>& items, bool absolute)
{
  String result;
  const List<String>::Node* i = items.getFirst();
  if(i)
  {
    result = translatePath(i->data, absolute);
    for(i = i->getNext(); i; i = i->getNext())
    {
      result.append(' ');
      result.append(translatePath(i->data, absolute));
    }
  }
  return result;
}

String CMake::translatePath(const String& path, bool absolute)
{
  if(::File::isPathAbsolute(path))
    return ::File::simplifyPath(path);
  return (absolute ? String("${CMAKE_CURRENT_SOURCE_DIR}/../../") : String("../../")) + ::File::simplifyPath(path);
}
