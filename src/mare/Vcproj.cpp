
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>

#include "Tools/Word.h"
#include "Tools/File.h"
#include "Tools/md5.h"
#include "Tools/Assert.h"
#include "Tools/Error.h"
#include "Engine.h"

#include "Vcproj.h"

bool Vcproj::generate(const Map<String, String>& userArgs)
{
  //knownCppOptions.append("/Z7", Option("DebugInformationFormat", "OldStyle"));
  //knownLinkOptions.append("/DEBUG", Option("GenerateDebugInformation", "true"));

  //
  engine.addDefaultKey("tool", "vcxproj");
  engine.addDefaultKey("vcxproj", "vcxproj");
  engine.addDefaultKey("host", "Win32");
  engine.addDefaultKey("platforms", "Win32");
  engine.addDefaultKey("configurations", "Debug Release");
  engine.addDefaultKey("targets"); // an empty target list exists per default
  engine.addDefaultKey("buildDir", "$(configuration)");
  engine.addDefaultKey("cppFlags", "/W3 $(if $(Debug),,/O2 /Oy)");
  engine.addDefaultKey("linkFlags", "$(if $(Debug),/INCREMENTAL /DEBUG,/OPT:REF /OPT:ICF)");
  {
    Map<String, String> cSource;
    cSource.append("command", "__clCompile");
    engine.addDefaultKey("cppSource", cSource);
    engine.addDefaultKey("cSource", cSource);
  }
  {
    Map<String, String> rcSource;
    rcSource.append("command", "__rcCompile");
    engine.addDefaultKey("rcSource", rcSource);
  }
  {
    Map<String, String> cApplication;
    cApplication.append("command", "__Application");
    cApplication.append("output", "$(buildDir)/$(target).exe");
    engine.addDefaultKey("cppApplication", cApplication);
    engine.addDefaultKey("cApplication", cApplication);
  }
  {
    Map<String, String> cDynamicLibrary;
    cDynamicLibrary.append("command", "__DynamicLibrary");
    cDynamicLibrary.append("output", "$(buildDir)/$(patsubst lib%,%,$(target)).dll");
    engine.addDefaultKey("cppDynamicLibrary", cDynamicLibrary);
    engine.addDefaultKey("cDynamicLibrary", cDynamicLibrary);
  }
  {
    Map<String, String> cStaticLibrary;
    cStaticLibrary.append("command", "__StaticLibrary");
    cStaticLibrary.append("output", "$(buildDir)/$(patsubst lib%,%,$(target)).lib");
    engine.addDefaultKey("cppStaticLibrary", cStaticLibrary);
    engine.addDefaultKey("cStaticLibrary", cStaticLibrary);
  }

  // add user arguments
  for(const Map<String, String>::Node* i = userArgs.getFirst(); i; i = i->getNext())
    engine.addCommandLineKey(i->key, i->data);

  // step #1: read input file
  if(!readFile())
    return false;

  // step #2: ...
  if(!processData())
    return false;

  // step #3: generate solution and project files
  if(!generateSln())
    return false;
  if(!generateVcprojs())
    return false;

  return true;
}

bool Vcproj::readFile()
{
  // get some global keys
  engine.enterRootKey();
  solutionName = engine.getFirstKey("name");
  List<String> allConfigurations, allTargets;
  engine.getKeys("platforms", platforms);
  engine.getKeys("configurations", allConfigurations);
  engine.getKeys("targets", allTargets);
  engine.leaveKey();

  // do something for each target in each configuration
  for(const List<String>::Node* i = platforms.getFirst(); i; i = i->getNext())
  {
    const String& platform = i->data;
    for(const List<String>::Node* i = allConfigurations.getFirst(); i; i = i->getNext())
    {
      const String& configName = i->data;
      const String configKey = configName + "|" + platform;
      const Config& config = configs.append(configKey, Config(configName, platform));

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
        Project& project = node ? node->data : projects.append(i->data, Project(i->data, createSomethingLikeGUID(i->data)));
        Project::Config& projectConfig = project.configs.append(configKey, Project::Config(config.name, config.platform));

        if(project.displayName.isEmpty())
          project.displayName = engine.getFirstKey("name", false);
        if(project.filter.isEmpty())
          project.filter = engine.getFirstKey("folder", false);

        engine.getText("buildCommand", projectConfig.buildCommand, false);
        engine.getText("reBuildCommand", projectConfig.reBuildCommand, false);
        engine.getText("cleanCommand", projectConfig.cleanCommand, false);
        engine.getText("preBuildCommand", projectConfig.preBuildCommand, false);
        engine.getText("preLinkCommand", projectConfig.preLinkCommand, false);
        engine.getText("postBuildCommand", projectConfig.postBuildCommand, false);
        projectConfig.buildDir = engine.getFirstKey("buildDir", true);

        engine.getText("message", projectConfig.message, false);
        engine.getText("command", projectConfig.command, false);
        engine.getKeys("output", projectConfig.outputs, false);
        engine.getKeys("input", projectConfig.inputs, false);

        List<String> cAndCppFlags;
        engine.getKeys("cppFlags", cAndCppFlags, true);
        engine.getKeys("cFlags", cAndCppFlags, true);
        for(const List<String>::Node* i = cAndCppFlags.getFirst(); i; i = i->getNext())
          projectConfig.cAndCppFlags.append(i->data, 0);
        List<String> linkFlags;
        engine.getKeys("linkFlags", linkFlags, true);
        for(const List<String>::Node* i = linkFlags.getFirst(); i; i = i->getNext())
          projectConfig.linkFlags.append(i->data, 0);
        projectConfig.firstOutput = engine.getFirstKey("output", false);
        engine.getKeys("defines", projectConfig.defines, true);
        engine.getKeys("includePaths", projectConfig.includePaths, true);
        engine.getKeys("libPaths", projectConfig.libPaths, true);
        engine.getKeys("libs", projectConfig.libs, true);
        engine.getKeys("dependencies", projectConfig.dependencies, false);
        engine.getKeys("root", project.root, true);

        if(engine.enterKey("files"))
        {
          List<String> files;
          engine.getKeys(files);
          for(const List<String>::Node* i = files.getFirst(); i; i = i->getNext())
          {
            Map<String, Project::File>::Node* node = project.files.find(i->data);
            Project::File& file = node ? node->data : project.files.append(i->data);
            Project::File::Config& fileConfig = file.configs.append(configKey);

            engine.enterUnnamedKey();
            engine.addDefaultKey("file", i->data);
            VERIFY(engine.enterKey(i->data));
            engine.getText("command", fileConfig.command, false);
            file.filter = engine.getFirstKey("folder", false);
            engine.getText("message", fileConfig.message, false);
            engine.getKeys("output", fileConfig.outputs, false);
            engine.getKeys("input", fileConfig.inputs, false);
            engine.getKeys("dependencies", fileConfig.dependencies, false);
            List<String> cAndCppFlags;
            engine.getKeys("cppFlags", cAndCppFlags, false);
            engine.getKeys("cFlags", cAndCppFlags, false);
            for(const List<String>::Node* i = cAndCppFlags.getFirst(); i; i = i->getNext())
              fileConfig.cAndCppFlags.append(i->data, 0);
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

bool Vcproj::processData()
{
  // prepare project and file types
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    Project& project = i->data;

    // handle project filter (solution explorer folder)
    if(!project.filter.isEmpty())
    {
      List<String> filtersToAdd;
      String filterName = project.filter;
      filterName.subst("/", "\\");
      for(;;)
      {
        filtersToAdd.prepend(filterName);
        filterName = File::getDirname(filterName);
        if(filterName == ".")
          break;
      }
      ProjectFilter* parentFilter = 0;
      for(List<String>::Node* i = filtersToAdd.getFirst(); i; i = i->getNext())
      {
        String& filterName = i->data;
        Map<String, ProjectFilter>::Node* node = projectFilters.find(filterName);
        ProjectFilter& filter = node ? node->data : projectFilters.append(filterName, ProjectFilter(createSomethingLikeGUID(filterName)));
        if(parentFilter)
        {
          parentFilter->filters.append(&filter);
        }
        if(!i->getNext())
        {
          filter.projects.append(&project);
        }
        parentFilter = &filter;
      }
    }

    // for each configuation
    for(Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
    {
      Project::Config& projectConfig = i->data;

      // determine project type
      projectConfig.type = "Utility";
      if(!projectConfig.command.isEmpty())
      {
        const String& firstCommandWord = Word::first(projectConfig.command.getFirst()->data);
        if(firstCommandWord == "__Custom")
        {
          projectConfig.type = "Makefile";
          projectConfig.command.clear();
        }
        else if(firstCommandWord == "__Application" || firstCommandWord == "__StaticLibrary" || firstCommandWord == "__DynamicLibrary")
        {
          projectConfig.type = firstCommandWord.substr(2);
          projectConfig.command.clear();
        }
      }
      if(!projectConfig.buildCommand.isEmpty())
      {
        projectConfig.type = "Makefile";
        projectConfig.command.clear();
      }

      // add dependencies of this project configuration to the (global) project's dependencies
      for(const List<String>::Node* i = projectConfig.dependencies.getFirst(); i; i = i->getNext())
        if(!project.dependencies.find(i->data))
          project.dependencies.append(i->data);

      // prepare configuration option list
      {
        Map<String, void*> defines;
        for(List<String>::Node* i = projectConfig.defines.getFirst(); i; i = i->getNext())
          defines.append(i->data, 0);
        if(defines.find("UNICODE") || defines.find("_UNICODE"))
          projectConfig.vsOptions.append("CharacterSet", "1");
        else if(defines.find("_MBCS"))
          projectConfig.vsOptions.append("CharacterSet", "2");
        else
          projectConfig.vsOptions.append("CharacterSet", "0");
      }

      // prepare c/cpp option list
      {
        List<String> additionalOptions;
        for(const Map<String, void*>::Node* i = projectConfig.cAndCppFlags.getFirst(); i; i = i->getNext())
          if(!knownCppOptions.find(i->key))
            additionalOptions.append(i->key);
        if(!additionalOptions.isEmpty())
          projectConfig.cppOptions.append("AdditionalOptions", join(additionalOptions, ' '));

        if(!projectConfig.includePaths.isEmpty())
          projectConfig.cppOptions.append("AdditionalIncludeDirectories", join(projectConfig.includePaths));
        if(!projectConfig.defines.isEmpty())
          projectConfig.cppOptions.append("PreprocessorDefinitions", join(projectConfig.defines));

        for(const Map<String, void*>::Node* i = projectConfig.cAndCppFlags.getFirst(); i; i = i->getNext())
        {
          const Map<String, Option>::Node* node = knownCppOptions.find(i->key);
          if(node)
          {
            const Option& option = node->data;
            if(!option.name.isEmpty())
              if(projectConfig.cppOptions.find(option.name))
              {
                // TODO: warning or error
              }
              else
                projectConfig.cppOptions.append(option.name, option.value);
          }
        }
      }

      // prepare VCLibrarianTool option list
      if(projectConfig.type == "StaticLibrary")
      {
        /*
        List<String> additionalOptions;
        for(const Map<String, void*>::Node* i = projectConfig.linkFlags.getFirst(); i; i = i->getNext())
          if(!knownLinkOptions.find(i->key))
            additionalOptions.append(i->key);
        if(!additionalOptions.isEmpty())
          projectConfig.librarianOptions.append("AdditionalOptions", join(additionalOptions, ' '));
          */

        if(!projectConfig.libs.isEmpty())
          projectConfig.librarianOptions.append("AdditionalDependencies", join(projectConfig.libs, ' ', ".lib"));
        if(!projectConfig.firstOutput.isEmpty())
        {
          String path = projectConfig.firstOutput;
          path.subst("/", "\\");
          projectConfig.librarianOptions.append("OutputFile", path);
        }
        if(!projectConfig.libPaths.isEmpty())
          projectConfig.librarianOptions.append("AdditionalLibraryDirectories", join(projectConfig.libPaths));
        /*
        for(const Map<String, void*>::Node* i = projectConfig.linkFlags.getFirst(); i; i = i->getNext())
        {
          const Map<String, Option>::Node* node = knownLinkOptions.find(i->key);
          if(node)
          {
            const Option& option = node->data;
            if(!option.name.isEmpty())
              if(projectConfig.linkOptions.find(option.name))
              {
                // TODO: warning or error
              }
              else
                projectConfig.librarianOptions.append(option.name, option.value);
          }
        }
        */
      }

      // prepare link option list
      if(projectConfig.type == "Application" || projectConfig.type == "DynamicLibrary")
      {
        List<String> additionalOptions;
        for(const Map<String, void*>::Node* i = projectConfig.linkFlags.getFirst(); i; i = i->getNext())
          if(!knownLinkOptions.find(i->key))
            additionalOptions.append(i->key);
        if(!additionalOptions.isEmpty())
          projectConfig.linkOptions.append("AdditionalOptions", join(additionalOptions, ' '));

        if(!projectConfig.libs.isEmpty())
          projectConfig.linkOptions.append("AdditionalDependencies", join(projectConfig.libs, ' ', ".lib"));
        if(!projectConfig.firstOutput.isEmpty())
        {
          String path = projectConfig.firstOutput;
          path.subst("/", "\\");
          projectConfig.linkOptions.append("OutputFile", path);
        }
        if(!projectConfig.libPaths.isEmpty())
          projectConfig.linkOptions.append("AdditionalLibraryDirectories", join(projectConfig.libPaths));

        for(const Map<String, void*>::Node* i = projectConfig.linkFlags.getFirst(); i; i = i->getNext())
        {
          const Map<String, Option>::Node* node = knownLinkOptions.find(i->key);
          if(node)
          {
            const Option& option = node->data;
            if(!option.name.isEmpty())
              if(projectConfig.linkOptions.find(option.name))
              {
                // TODO: warning or error
              }
              else
                projectConfig.linkOptions.append(option.name, option.value);
          }
        }
      }
    }

    // for each file
    for(Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
    {
      Project::File& file = i->data;

      for(Map<String, Project::File::Config>::Node* i = file.configs.getFirst(); i; i = i->getNext())
      {
        Project::File::Config& fileConfig = i->data;
        Project::Config& projectConfig = project.configs.find(i->key)->data;

        // determine file type
        String firstCommandWord = fileConfig.command.isEmpty() ? String() : Word::first(fileConfig.command.getFirst()->data);
        String type;
        if(firstCommandWord == "__clCompile")
          type = "ClCompile";
        else if(firstCommandWord == "__rcCompile")
          type = "ResourceCompile";
        else if(!firstCommandWord.isEmpty())
          type = "CustomBuild";

        if(!type.isEmpty())
        {
          if(!file.type.isEmpty() && file.type != type)
          { // the file type must be consistent over with the other configurations
            // TODO: warning or error?
          }
          else
            file.type = type;
        }

        // add dependencies of the file to project's dependencies
        for(const List<String>::Node* i = fileConfig.dependencies.getFirst(); i; i = i->getNext())
          if(!project.dependencies.find(i->data))
            project.dependencies.append(i->data);

        // prepare cpp option list
        {
          List<String> additionalOptions;
          for(const Map<String, void*>::Node* i = fileConfig.cAndCppFlags.getFirst(); i; i = i->getNext())
            if(!projectConfig.cAndCppFlags.find(i->key) && !knownCppOptions.find(i->key))
              additionalOptions.append(i->key);
          if(!additionalOptions.isEmpty())
            fileConfig.cppOptions.append("AdditionalOptions", join(additionalOptions, ' '));

          for(const Map<String, void*>::Node* i = fileConfig.cAndCppFlags.getFirst(); i; i = i->getNext())
            if(!projectConfig.cAndCppFlags.find(i->key))
            {
              const Map<String, Option>::Node* node = knownCppOptions.find(i->key);
              if(node)
              {
                const Option& option = node->data;
                if(!option.name.isEmpty())
                  if(fileConfig.cppOptions.find(option.name))
                  {
                    // TODO: warning or error
                  }
                  else
                    fileConfig.cppOptions.append(option.name, option.value);
              }
            }

            /*
          if(!fileConfig.cAndCppFlags.isEmpty())
            for(const Map<String, void*>::Node* i = projectConfig.cAndCppFlags.getFirst(); i; i = i->getNext())
              if(!fileConfig.cAndCppFlags.find(i->key))
              {
                const Map<String, Option>::Node* node = knownCppOptions.find(i->key);
                if(node)
                {
                    const Option& option = node->data;
                    if(!option.unsetValue.isEmpty() && !fileConfig.cppOptions.find(i->key))
                      fileConfig.cppOptions.append(option.name, option.unsetValue);
                }
              }
              */

          if(!fileConfig.cppOptions.isEmpty())
            file.useDefaultSettings = false;
        }

      }

      //
      if(file.configs.getSize() < project.configs.getSize())
        file.useDefaultSettings = false;
    }

    // set special file type for header files
    for(Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
      if(i->data.type.isEmpty())
      {
        String extension = File::getExtension(i->key);
        if(extension == "h" || extension == "hh" || extension == "hxx"  || extension == "hpp")
          i->data.type = "ClInclude";
        else
          i->data.type = "None";
      }
  }

  // resolve dependencies 
  if(!resolveDependencies())
    return false;

  return true;
}

bool Vcproj::resolveDependencies()
{
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    Project& project = i->data;

    for(Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
    {
      const String& configKey = i->key;
      Project::Config& config = i->data;
      if(config.command.isEmpty())
        continue;

      // create a CustomBuild rule to build the target
      Project::File* file = 0;
      for(const List<String>::Node* i = config.inputs.getFirst(); i; i = i->getNext())
      { // try using an input file that was added by the user
        Map<String, Project::File>::Node* node = project.files.find(i->data);
        if(node)
        {
          file = &node->data;
          if(file->type == "None" || file->type == "ClInclude")
            goto foundFile;
          if(file->type == "CustomBuild")
          {
            const Map<String, Project::File::Config>::Node* node = file->configs.find(configKey);
            if(!node)
              goto foundFile;
            const Project::File::Config& fileConfig = node->data;
            if(fileConfig.command.isEmpty() && fileConfig.outputs.isEmpty())
              goto foundFile;
          }
        }
      }
      for(const List<String>::Node* i = config.inputs.getFirst(); i; i = i->getNext())
      { // try using any unused input file
        Map<String, Project::File>::Node* node = project.files.find(i->data);
        if(!node)
        {
          file = &project.files.append(i->data);
          goto foundFile;
        }
      }
      // TODO: error message
      return false;

    foundFile:
      file->type = "CustomBuild";
      Map<String, Project::File::Config>::Node* node = file->configs.find(configKey);
      Project::File::Config& fileConfig = node ? node->data : file->configs.append(configKey);
      fileConfig.command = config.command;
      fileConfig.dependencies = config.dependencies;
      fileConfig.inputs = config.inputs;
      fileConfig.message = config.message;
      fileConfig.outputs = config.outputs;
    }

    for(Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
    {
      Project::File& file = i->data;
      if(file.type != "CustomBuild")
        continue;

      for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
      {
        const String& configKey = i->key;
        Map<String, Project::File::Config>::Node* node = file.configs.find(i->key);
        if(!node)
          continue;

        Project::File::Config& fileConfig = node->data;

        // resolve target dependencies (add outputs of the dependencies to the list of input files)
        for(const List<String>::Node* i = fileConfig.dependencies.getFirst(); i; i = i->getNext())
        {
          const Map<String, Project>::Node* node = projects.find(i->data);
          if(node)
          {
            const Project& depProj = node->data;
            const Map<String, Project::Config>::Node* node = depProj.configs.find(configKey);
            if(node)
              for(const List<String>::Node* i = node->data.outputs.getFirst(); i; i = i->getNext())
                fileConfig.inputs.append(i->data);
          }
        }
      }
    }
  }
  return true;
}

bool Vcproj::generateSln()
{
  // find "active" projects (first project and all its dependencies)
  Map<String, void*> buildProjects;
  if(!projects.isEmpty())
  {
    struct A
    {
      static void addProjectDeps(const Project& project, Vcproj& vcxproj, Map<String, void*>& buildProjects)
      {
        buildProjects.append(project.name);
        for(const Map<String, void*>::Node* i = project.dependencies.getFirst(); i; i = i->getNext())
          if(!buildProjects.find(i->key))
          {
            Map<String, Project>::Node* subproject = vcxproj.projects.find(i->key);
            if(subproject)
              addProjectDeps(subproject->data, vcxproj, buildProjects);
          }
      }
    };
    A::addProjectDeps(projects.getFirst()->data, *this, buildProjects);
  }

  // remove projects not creating any output files
  for(Map<String, Project>::Node* i = projects.getFirst(), * nexti; i; i = nexti)
  {
    nexti = i->getNext();
    if(!i->data.files.isEmpty())
      continue;
    for(const Map<String, Project::Config>::Node* j = i->data.configs.getFirst(); j; j = j->getNext())
      if(!j->data.command.isEmpty() || !j->data.outputs.isEmpty() || j->data.type != "Utility")
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
  if(solutionName.isEmpty() && !projects.isEmpty())
    solutionName = projects.getFirst()->data.name;

  // open output file
  fileOpen(solutionName + ".sln");

  // header
  fileWrite("﻿\r\n");
  fileWrite("Microsoft Visual Studio Solution File, Format Version 10.00\r\n");
  fileWrite("# Visual Studio 2008\r\n");

  // project list
  for(const Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    fileWrite(String("Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"") + i->key + "\", \"" + i->key + ".vcproj\", \"{" + i->data.guid + "}\"\r\n");
    fileWrite("EndProject\r\n");
  }

  // project filter list
  for(const Map<String, ProjectFilter>::Node* i = projectFilters.getFirst(); i; i = i->getNext())
  {
    String name = File::getBasename(i->key);
    fileWrite(String("Project(\"{2150E333-8FDC-42A3-9474-1A3956D46DE8}\") = \"") + name + "\", \"" + name + "\", \"{" + i->data.guid + "}\"\r\n");
    fileWrite("EndProject\r\n");
  }

  //
  fileWrite("Global\r\n");

  // solution configuration list
  fileWrite("\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\r\n");
  for(const Map<String, Config>::Node* i = configs.getFirst(); i; i = i->getNext())
    fileWrite(String("\t\t") + i->key + " = " + i->key + "\r\n");
  fileWrite("\tEndGlobalSection\r\n");

  // solution config to project config map
  fileWrite("\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\r\n");
  for(const Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    const Project& project = i->data;
    for(const Map<String, Config>::Node* i = configs.getFirst(); i; i = i->getNext())
    {
      const String& configKey = i->key;
      const Config& config = i->data;
      String projectConfigKey;
      if(project.configs.find(configKey))
        projectConfigKey = configKey;
      else
        projectConfigKey = project.configs.getFirst()->data.name + "|" + config.platform;
      fileWrite(String("\t\t{") + project.guid + "}." + configKey + ".ActiveCfg = " + projectConfigKey + "\r\n");
      if(buildProjects.isEmpty() || buildProjects.find(project.name))
        fileWrite(String("\t\t{") + project.guid + "}." + configKey + ".Build.0 = " + projectConfigKey + "\r\n");
    }
  }
  fileWrite("\tEndGlobalSection\r\n");

  //
  fileWrite("\tGlobalSection(SolutionProperties) = preSolution\r\n");
  fileWrite("\t\tHideSolutionNode = FALSE\r\n");
  fileWrite("\tEndGlobalSection\r\n");

  // add projects to project filters
  if(!projectFilters.isEmpty())
  {
    fileWrite("\tGlobalSection(NestedProjects) = preSolution\r\n");
    for(const Map<String, ProjectFilter>::Node* i = projectFilters.getFirst(); i; i = i->getNext())
    {
      const ProjectFilter& filter = i->data;
      for(const List<Project*>::Node* i = filter.projects.getFirst(); i; i = i->getNext())
      {
        const Project& project = *i->data;
        fileWrite(String("\t\t{") + project.guid + "} = {" + filter.guid + "}\r\n");
      }
      for(const List<ProjectFilter*>::Node* i = filter.filters.getFirst(); i; i = i->getNext())
      {
        const ProjectFilter& childFilter = *i->data;
        fileWrite(String("\t\t{") + childFilter.guid + "} = {" + filter.guid + "}\r\n");
      }
    }
    fileWrite("\tEndGlobalSection\r\n");
  }

  //
  fileWrite("EndGlobal\r\n");

  fileClose();
  return true;
}

bool Vcproj::generateVcprojs()
{
  for(const Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
    if(!generateVcproj(i->data))
      return false;
  return true;
}

bool Vcproj::generateVcproj(const Project& project)
{
  fileOpen(project.name + ".vcproj");

  fileWrite("<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\r\n");
  fileWrite(String("<VisualStudioProject\r\n"
            "\tProjectType=\"Visual C++\"\r\n"
            "\tVersion=\"9,00\"\r\n"
            "\tName=\"") + (project.displayName.isEmpty() ? project.name : project.displayName) + "\"\r\n"
            "\tProjectGUID=\"{" + project.guid + "}\"\r\n"
            "\tRootNamespace=\"" + project.name + "\"\r\n"
            "\tKeyword=\"Win32Proj\"\r\n"
            "\tTargetFrameworkVersion=\"196613\"\r\n"
            "\t>\r\n");

  fileWrite("\t<Platforms>\r\n");
  for(List<String>::Node* i = platforms.getFirst(); i; i = i->getNext())
  {
    fileWrite("\t\t<Platform\r\n");
    fileWrite(String("\t\t\tName=\"") + i->data + "\"\r\n");
    fileWrite("\t\t/>\r\n");
  }
  fileWrite("\t</Platforms>\r\n");

  fileWrite("\t<ToolFiles>\r\n");
  fileWrite("\t</ToolFiles>\r\n");

  fileWrite("\t<Configurations>\r\n");
  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    const Project::Config& config = i->data;
    fileWrite("\t\t<Configuration\r\n");
    fileWrite(String("\t\t\tName=\"") + i->key + "\"\r\n");
    if(!config.firstOutput.isEmpty())
    {
      String path = File::getDirname(config.firstOutput);
      path.subst("/", "\\");
      fileWrite(String("\t\t\tOutputDirectory=\"") + path + "\"\r\n");
    }
    if(!config.buildDir.isEmpty())
    {
      String path = config.buildDir;
      path.subst("/", "\\");
      if(config.firstOutput.isEmpty())
        fileWrite(String("\t\t\tOutputDirectory=\"") + path + "\"\r\n");
      fileWrite(String("\t\t\tIntermediateDirectory=\"") + path + "\"\r\n");
    }
    String configurationType("1");
    if(config.type == "StaticLibrary")
      configurationType = "4";
    else if(config.type == "DynamicLibrary")
      configurationType = "2";
    else if(config.type == "Makefile")
      configurationType = "0";
    else if(config.type == "Utility")
      configurationType = "10";
    fileWrite(String("\t\t\tConfigurationType=\"") + configurationType + "\"\r\n");
    for(const Map<String, String>::Node* i = config.vsOptions.getFirst(); i; i = i->getNext())
      fileWrite(String("\t\t\t") + i->key + "=\"" + i->data + "\"\r\n");
    fileWrite("\t\t\t>\r\n");
    if(config.type == "Makefile")
    {
      fileWrite("\t\t\t<Tool\r\n");
      fileWrite("\t\t\t\tName=\"VCNMakeTool\"\r\n");
      fileWrite("\t\t\t\tBuildCommandLine=\"\"\r\n");
      fileWrite("\t\t\t\tReBuildCommandLine=\"\"\r\n");
      fileWrite("\t\t\t\tCleanCommandLine=\"\"\r\n");
      fileWrite("\t\t\t\tOutput=\"\"\r\n");
      fileWrite("\t\t\t\tPreprocessorDefinitions=\"\"\r\n");
      fileWrite("\t\t\t\tIncludeSearchPath=\"\"\r\n");
      fileWrite("\t\t\t\tForcedIncludes=\"\"\r\n");
      fileWrite("\t\t\t\tAssemblySearchPath=\"\"\r\n");
      fileWrite("\t\t\t\tForcedUsingAssemblies=\"\"\r\n");
      fileWrite("\t\t\t\tCompileAsManaged=\"\"\r\n");
      fileWrite("\t\t\t/>\r\n");
    }
    else
    {
      fileWrite("\t\t\t<Tool\r\n");
      fileWrite("\t\t\t\tName=\"VCPreBuildEventTool\"\r\n");
      fileWrite("\t\t\t/>\r\n");
      fileWrite("\t\t\t<Tool\r\n");
      fileWrite("\t\t\t\tName=\"VCCustomBuildTool\"\r\n");
      fileWrite("\t\t\t/>\r\n");
      if(config.type != "Utility")
      {
        fileWrite("\t\t\t<Tool\r\n");
        fileWrite("\t\t\t\tName=\"VCXMLDataGeneratorTool\"\r\n");
        fileWrite("\t\t\t/>\r\n");
        fileWrite("\t\t\t<Tool\r\n");
        fileWrite("\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"\r\n");
        fileWrite("\t\t\t/>\r\n");
      }
      fileWrite("\t\t\t<Tool\r\n");
      fileWrite("\t\t\t\tName=\"VCMIDLTool\"\r\n");
      fileWrite("\t\t\t/>\r\n");
      if(config.type != "Utility")
      {
        fileWrite("\t\t\t<Tool\r\n");
        fileWrite("\t\t\t\tName=\"VCCLCompilerTool\"\r\n");
        for(const Map<String, String>::Node* i = config.cppOptions.getFirst(); i; i = i->getNext())
          fileWrite(String("\t\t\t\t") + i->key + "=\"" + i->data + "\"\r\n");
        fileWrite("\t\t\t/>\r\n");
        fileWrite("\t\t\t<Tool\r\n");
        fileWrite("\t\t\t\tName=\"VCManagedResourceCompilerTool\"\r\n");
        fileWrite("\t\t\t/>\r\n");
        fileWrite("\t\t\t<Tool\r\n");
        fileWrite("\t\t\t\tName=\"VCResourceCompilerTool\"\r\n");
        fileWrite("\t\t\t/>\r\n");
        fileWrite("\t\t\t<Tool\r\n");
        fileWrite("\t\t\t\tName=\"VCPreLinkEventTool\"\r\n");
        fileWrite("\t\t\t/>\r\n");
        if(config.type == "StaticLibrary")
        {
          fileWrite("\t\t\t<Tool\r\n");
          fileWrite("\t\t\t\tName=\"VCLibrarianTool\"\r\n");
          for(const Map<String, String>::Node* i = config.librarianOptions.getFirst(); i; i = i->getNext())
            fileWrite(String("\t\t\t\t") + i->key + "=\"" + i->data + "\"\r\n");
          fileWrite("\t\t\t/>\r\n");
        }
        else
        {
          fileWrite("\t\t\t<Tool\r\n");
          fileWrite("\t\t\t\tName=\"VCLinkerTool\"\r\n");
          for(const Map<String, String>::Node* i = config.linkOptions.getFirst(); i; i = i->getNext())
            fileWrite(String("\t\t\t\t") + i->key + "=\"" + i->data + "\"\r\n");
          //fileWrite("\t\t\t\tLinkIncremental=\"2\"\r\n");
          //fileWrite("\t\t\t\tGenerateDebugInformation=\"true\"\r\n");
          //fileWrite("\t\t\t\tSubSystem=\"1\"\r\n");
          //fileWrite("\t\t\t\tTargetMachine=\"1\"\r\n");
          fileWrite("\t\t\t/>\r\n");
        }
        fileWrite("\t\t\t<Tool\r\n");
        fileWrite("\t\t\t\tName=\"VCALinkTool\"\r\n");
        fileWrite("\t\t\t/>\r\n");
        if(config.type != "StaticLibrary")
        {
          fileWrite("\t\t\t<Tool\r\n");
          fileWrite("\t\t\t\tName=\"VCManifestTool\"\r\n");
          fileWrite("\t\t\t/>\r\n");
        }
        fileWrite("\t\t\t<Tool\r\n");
        fileWrite("\t\t\t\tName=\"VCXDCMakeTool\"\r\n");
        fileWrite("\t\t\t/>\r\n");
        fileWrite("\t\t\t<Tool\r\n");
        fileWrite("\t\t\t\tName=\"VCBscMakeTool\"\r\n");
        fileWrite("\t\t\t/>\r\n");
        fileWrite("\t\t\t<Tool\r\n");
        fileWrite("\t\t\t\tName=\"VCFxCopTool\"\r\n");
        fileWrite("\t\t\t/>\r\n");
        if(config.type != "StaticLibrary")
        {
          fileWrite("\t\t\t<Tool\r\n");
          fileWrite("\t\t\t\tName=\"VCAppVerifierTool\"\r\n");
          fileWrite("\t\t\t/>\r\n");
        }
      }
      fileWrite("\t\t\t<Tool\r\n");
      fileWrite("\t\t\t\tName=\"VCPostBuildEventTool\"\r\n");
      fileWrite("\t\t\t/>\r\n");
    }
    fileWrite("\t\t</Configuration>\r\n");
  }
  fileWrite("\t</Configurations>\r\n");

  fileWrite("\t<References>\r\n");
  for(const Map<String, void*>::Node* i = project.dependencies.getFirst(); i; i = i->getNext())
  {
    const Map<String, Project>::Node* node = projects.find(i->key);
    if(node)
    {
      const Project& project = node->data;
      fileWrite("\t\t<ProjectReference\r\n");
      fileWrite(String("\t\t\tReferencedProjectIdentifier=\"{") + project.guid + "}\"\r\n");
      fileWrite(String("\t\t\tRelativePathToProject=\".\\") + project.name + ".vcproj\"\r\n");
      fileWrite("\t\t/>\r\n");
    }
  }
  fileWrite("\t</References>\r\n");

  generateVcprojFiles(project);

  fileWrite("\t<Globals>\r\n");
  fileWrite("\t</Globals>\r\n");

  fileWrite("</VisualStudioProject>\r\n");

  fileClose();
  return true;
}

bool Vcproj::generateVcprojFiles(const Project& project)
{
  struct Filter
  {
    String name;
    List<String> files;
    List<Filter*> filters;

    void write(Vcproj& vc, const String& tabs = String())
    {
      for(List<Filter*>::Node* i = filters.getFirst(); i; i = i->getNext())
      {
        Filter& filter = *i->data;
        vc.fileWrite(tabs + "\t\t<Filter\r\n");
        vc.fileWrite(tabs + "\t\t\tName=\"" + filter.name + "\"\r\n");
        vc.fileWrite(tabs + "\t\t\t>\r\n");
        filter.write(vc, tabs + "\t");
        vc.fileWrite(tabs + "\t\t</Filter>\r\n");
      }
      for(const List<String>::Node* i = files.getFirst(); i; i = i->getNext())
      {
        vc.fileWrite(tabs + "\t\t<File\r\n");
        String path = i->data;
        path.subst("/", "\\");
        vc.fileWrite(tabs + "\t\t\tRelativePath=\"" + path + "\"\r\n");
        vc.fileWrite(tabs + "\t\t\t>\r\n");
        vc.fileWrite(tabs + "\t\t</File>\r\n");
      }
    }
  };

  Map<String, void*> rootPaths;
  for(const List<String>::Node* i = project.root.getFirst(); i; i = i->getNext())
  {
    String path = i->data;
    path.subst("/", "\\");
    rootPaths.append(path);
  }

  Map<String, Filter> filters;
  Filter root;

  for(const Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
  {
    const Project::File& file = i->data;
    const String& filePath = i->key;

    List<String> filtersToAdd;
    if(!file.filter.isEmpty()) // a user defined filter
    {
      String filterName = file.filter;
      filterName.subst("/", "\\");
      for(;;)
      {
        filtersToAdd.prepend(filterName);
        filterName = File::getDirname(filterName);
        if(filterName == ".")
          break;
      }
    }
    else // create a filter based on the file system hierarchy
    {
      String root;
      String filterName = File::getDirname(i->key);
      filterName.subst("/", "\\");
      for(;;)
      {
        if(filterName == "." || File::getBasename(filterName) == "..")
          break;
        const Map<String, void*>::Node* node = rootPaths.find(filterName);
        if(node)
        {
          root = node->key;
          break;
        }
        filtersToAdd.prepend(filterName);
        filterName = File::getDirname(filterName);
      }
      for(List<String>::Node* i = filtersToAdd.getFirst(); i; i = i->getNext())
      {
        String filterName = i->data;
        if(root.isEmpty())
        { // remove leading ../ and ./
          for(;;)
          {
            if(strncmp(filterName.getData(), "..\\", 3) == 0)
              filterName = filterName.substr(3);
            else if(strncmp(filterName.getData(), ".\\", 2) == 0)
              filterName = filterName.substr(2);
            else
              break;
          }
        }
        else // remove leading root path
          filterName.substr(root.getLength() + 1);
      }
    }

    if(filtersToAdd.isEmpty())
      root.files.append(filePath);
    else
    {
      Filter* parentFilter = &root;
      for(List<String>::Node* i = filtersToAdd.getFirst(); i; i = i->getNext())
      {
        Map<String, Filter>::Node* node = filters.find(i->data);
        Filter& filter = node ? node->data : filters.append(i->data);
        if(!i->getNext())
          filter.files.append(filePath);
        if(!node)
        {
          filter.name = File::getBasename(i->data);
          parentFilter->filters.append(&filter);
        }
        parentFilter = &filter;
      }
    }
  }

  fileWrite("\t<Files>\r\n");
  root.write(*this);
  fileWrite("\t</Files>\r\n");
  return true;
}

void Vcproj::fileOpen(const String& name)
{
  if(!file.open(name, File::writeFlag))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
  openedFile = name;
}

void Vcproj::fileWrite(const String& data)
{
  if(!file.write(data))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
}

void Vcproj::fileClose()
{
  file.close();
  if(!openedFile.isEmpty())
  {
    puts(openedFile.getData());
    fflush(stdout);
  }
  openedFile.clear();
}

String Vcproj::createSomethingLikeGUID(const String& name)
{
  // create something like this: 8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942

  MD5 md5;
  md5.update((const unsigned char*)name.getData(), name.getLength());
  unsigned char sum[16];
  md5.final(sum);

  String result;
  char* dest = result.getData(32 + 4);
  static const char* digits = "0123456789ABCDEF";
  const unsigned char* pos = sum;
  for(const unsigned char* end = pos + 4; pos < end; ++pos)
  {
    *(dest++) = digits[*pos >> 4];
    *(dest++) = digits[*pos & 0xf];
  }
  for(int i = 0; i < 3; ++i)
  {
    *(dest++) = '-';
    for(const unsigned char* end = pos + 2; pos < end; ++pos)
    {
      *(dest++) = digits[*pos >> 4];
      *(dest++) = digits[*pos & 0xf];
    }
  }
  *(dest++) = '-';
  for(const unsigned char* end = pos + 6; pos < end; ++pos)
  {
    *(dest++) = digits[*pos >> 4];
    *(dest++) = digits[*pos & 0xf];
  }
  result.setLength(32 + 4);
  return result;
}

String Vcproj::join(const List<String>& items, char sep, const String& suffix)
{
  String result;
  const List<String>::Node* i = items.getFirst();
  if(i)
  {
    for(const char* str = i->data.getData(); *str; ++str)
      if(isspace(*(unsigned char*)str))
      {
        result.append("&quot;");
        result.append(xmlEscape(i->data));
        result.append("&quot;");
        goto next;
      }
    result = xmlEscape(i->data);
  next: ;
    result.append(suffix);
    for(i = i->getNext(); i; i = i->getNext())
    {
      result.append(sep);
      for(const char* str = i->data.getData(); *str; ++str)
        if(isspace(*(unsigned char*)str))
        {
          result.append("&quot;");
          result.append(xmlEscape(i->data));
          result.append("&quot;");
          goto next2;
        }
      result.append(xmlEscape(i->data));
    next2: ;
      result.append(suffix);
    }
  }
  return result;
}

String Vcproj::joinCommands(const List<String>& commands)
{
  String result;
  for(const List<String>::Node* j = commands.getFirst(); j; j = j->getNext())
  {
    if(j->data.isEmpty())
      continue;
    if(!result.isEmpty())
      result.append("&#x0D;&#x0A;");

    List<Word> commands;
    Word::split(j->data, commands);
    const List<Word>::Node* i = commands.getFirst();
    if(i)
    {
      String program(i->data);
      program.subst("/", "\\");
      for(const char* str = program.getData(); *str; ++str)
        if(isspace(*(unsigned char*)str))
        {
          result.append("&quot;");
          result.append(xmlEscape(program));
          result.append("&quot;");
          goto next;
        }
        result.append(xmlEscape(program));
      next: ;
      for(i = i->getNext(); i; i = i->getNext())
      {
        result.append(' ');
        for(const char* str = i->data.getData(); *str; ++str)
          if(isspace(*(unsigned char*)str))
          {
            result.append("&quot;");
            result.append(xmlEscape(i->data));
            result.append("&quot;");
            goto next2;
          }
        result.append(xmlEscape(i->data));
      next2: ;
      }
    }
  }
  return result;
  // TODO: something for more than a single command?
}

String Vcproj::xmlEscape(const String& text)
{
  const char* str = text.getData();
  for(; *str; ++str)
    if(*str == '<' || *str == '>' || *str == '&')
      goto escape;
  return text;
escape:
  String result(text);
  result.setLength(str - text.getData());
  for(; *str; ++str)
    switch(*str)
    {
    case '"': result.append("&quot;"); break;
    case '<': result.append("&lt;"); break;
    case '>': result.append("&gt;"); break;
    case '&': result.append("&amp;"); break;
    default: result.append(*str); break;
    }
  return result;
}