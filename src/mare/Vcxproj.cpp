
#include <cstring>
#include <cstdlib>

#include "Tools/Words.h"
#include "Tools/File.h"
#include "Tools/md5.h"
#include "Tools/Assert.h"
#include "Engine.h"

#include "Vcxproj.h"

bool Vcxproj::generate(const Map<String, String>& userArgs)
{
  // general tab:
  knownCppOptions.append("/Z7", Option("DebugInformationFormat", "OldStyle"));
  knownCppOptions.append("/Zi", Option("DebugInformationFormat", "ProgramDatabase"));
  knownCppOptions.append("/ZI", Option("DebugInformationFormat", "EditAndContinue"));
  // TODO: common language runtime support
  // TODO: suppress startup banner
  knownCppOptions.append("/W0", Option("WarningLevel", "TurnOffAllWarnings"));
  knownCppOptions.append("/W1", Option("WarningLevel", "Level1"));
  knownCppOptions.append("/W2", Option("WarningLevel", "Level2"));
  knownCppOptions.append("/W3", Option("WarningLevel", "Level3"));
  knownCppOptions.append("/W4", Option("WarningLevel", "Level4"));
  knownCppOptions.append("/Wall", Option("WarningLevel", "EnableAllWarnings"));
  knownCppOptions.append("/WX", Option("TreatWarningAsError", "true"));
  knownCppOptions.append("/WX-", Option("TreatWarningAsError", "false"));
  knownCppOptions.append("/MP", Option("MultiProcessorCompilation", "true"));
  // TODO: use unicode for assembler listing

  // optimization tab:
  knownCppOptions.append("/O1", Option("Optimization", "MinSize"));
  knownCppOptions.append("/O2", Option("Optimization", "MaxSpeed"));
  knownCppOptions.append("/Od", Option("Optimization", "Disabled"));
  knownCppOptions.append("/Ox", Option("Optimization", "Full"));
  knownCppOptions.append("/Ob2", Option("InlineFunctionExpansion", "AnySuitable"));
  knownCppOptions.append("/Ob1", Option("InlineFunctionExpansion", "OnlyExplicitInline"));
  knownCppOptions.append("/Ob0", Option("InlineFunctionExpansion", "Disabled"));
  knownCppOptions.append("/Oi", Option("IntrinsicFunctions", "true"));
  knownCppOptions.append("/Os", Option("FavorSizeOrSpeed", "Size"));
  knownCppOptions.append("/Ot", Option("FavorSizeOrSpeed", "Speed"));
  knownCppOptions.append("/Oy", Option("OmitFramePointers", "true"));
  knownCppOptions.append("/Oy-", Option("OmitFramePointers", "false"));
  knownCppOptions.append("/GT", Option("EnableFiberSafeOptimizations", "true"));
  knownCppOptions.append("/GL", Option("WholeProgramOptimization", "true"));

  // TODO: Preprocessor tab

  // code generation tab:
  knownCppOptions.append("/GF", Option("StringPooling", "true"));
  knownCppOptions.append("/Gm", Option("MinimalRebuild", "true"));
  knownCppOptions.append("/Gm-", Option("MinimalRebuild", "false"));
  // TODO: C++ exceptions
  // TODO: smaller type checks
  // TODO: basic runtime checks
  knownCppOptions.append("/MT", Option("RuntimeLibrary", "MultiThreaded"));
  knownCppOptions.append("/MD", Option("RuntimeLibrary", "MultiThreadedDLL"));
  knownCppOptions.append("/MTd", Option("RuntimeLibrary", "MultiThreadedDebug"));
  knownCppOptions.append("/MDd", Option("RuntimeLibrary", "MultiThreadedDebugDLL"));
  knownCppOptions.append("/GS", Option("BufferSecurityCheck", "true"));
  knownCppOptions.append("/GS-", Option("BufferSecurityCheck", "false"));
  knownCppOptions.append("/Gy", Option("FunctionLevelLinking", "true"));
  knownCppOptions.append("/Gy-", Option("FunctionLevelLinking", "false"));
  knownCppOptions.append("/fp:precise", Option("FloatingPointModel", "Precise"));
  knownCppOptions.append("/fp:strict", Option("FloatingPointModel", "Strict"));
  knownCppOptions.append("/fp:fast", Option("FloatingPointModel", "Fast"));
  // TODO: floating point exceptions
  // TODO: create hotpatchable image

  // language tab
  // TODO: disable language extensions
  knownCppOptions.append("/Zc:wchar_t", Option("TreatWChar_tAsBuiltInType", "true"));
  knownCppOptions.append("/Zc:wchar_t-", Option("TreatWChar_tAsBuiltInType", "false"));
  // TODO: force conformance in for loop scope
  // TODO: enable rtti
  // TODO: openmp

  // TODO: pch tab
  // TODO: output files tab?
  // TODO: browse information tab?
  // TODO: advanced tab?

  // TODO: serveral linker tabs?

  // debugging tab:
  knownLinkOptions.append("/DEBUG", Option("GenerateDebugInformation", "true"));  
  // TODO: more options?

  // system tab:
  knownLinkOptions.append("/SUBSYSTEM:CONSOLE", Option("SubSystem", "Console"));
  knownLinkOptions.append("/SUBSYSTEM:WINDOWS", Option("SubSystem", "Windows"));
  // TODO: more SubSystems
  // TODO: more system tab options?

  // optimization tab:
  knownLinkOptions.append("/INCREMENTAL");
  knownLinkOptions.append("/OPT:REF", Option("OptimizeReferences", "true"));
  knownLinkOptions.append("/OPT:NOREF", Option("OptimizeReferences", "false"));
  knownLinkOptions.append("/OPT:ICF", Option("EnableCOMDATFolding", "true"));
  knownLinkOptions.append("/OPT:NOICF", Option("EnableCOMDATFolding", "false"));
  //knownLinkOptions.append("/LTCG", Option("LinkTimeCodeGeneration", "UseLinkTimeCodeGeneration"));
  knownLinkOptions.append("/LTCG");
  // TODO: more LTCG options?
  // TODO: more options?
  
  engine.addDefaultKey("tool", "vcxproj");
  engine.addDefaultKey("vcxproj", "vcxproj");
  engine.addDefaultKey("host", "Win32");
  engine.addDefaultKey("platforms", "Win32");
  engine.addDefaultKey("configurations", "Debug Release");
  engine.addDefaultKey("targets");
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
    cApplication.append("outputs", "$(buildDir)/$(target).exe");
    engine.addDefaultKey("cppApplication", cApplication);
    engine.addDefaultKey("cApplication", cApplication);
  }
  {
    Map<String, String> cDynamicLibrary;
    cDynamicLibrary.append("command", "__DynamicLibrary");
    cDynamicLibrary.append("outputs", "$(buildDir)/$(patsubst lib%,%,$(target)).dll");
    engine.addDefaultKey("cppDynamicLibrary", cDynamicLibrary);
    engine.addDefaultKey("cDynamicLibrary", cDynamicLibrary);
  }
  {
    Map<String, String> cStaticLibrary;
    cStaticLibrary.append("command", "__StaticLibrary");
    cStaticLibrary.append("outputs", "$(buildDir)/$(patsubst lib%,%,$(target)).lib");
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
  if(!generateSln())
    return false;

  // generate project files
  if(!generateVcxprojs())
    return false;

  return true;
}

bool Vcxproj::readFile()
{

  // get some global keys
  solutionName = engine.getFirstKey("name");
  
  List<String> inputPlatforms, inputConfigurations, inputTargets;
  engine.getKeys("platforms", inputPlatforms);
  engine.getKeys("configurations", inputConfigurations);
  engine.getKeys("targets", inputTargets);

  engine.enterKey("platforms");

  for(const List<String>::Node* i = inputPlatforms.getFirst(); i; i = i->getNext())
  {
    const String& platform = i->data;
    engine.enterKey(platform);
    engine.addDefaultKey("platform", platform);
    engine.addDefaultKey(platform, platform);

    // enter configurations space
    engine.enterKey("configurations");

    // get configuration project list
    for(const List<String>::Node* i = inputConfigurations.getFirst(); i; i = i->getNext())
    {
      const String& configName = i->data;
      const String configKey = configName + "|" + platform;
      const Config& config = configs.append(configKey, Config(configName, platform));

      if(!engine.enterKey(configName))
      {
        engine.error(String().format(256, "cannot find configuration \"%s\"", configName.getData()));
        return false;
      }
      engine.addDefaultKey("configuration", configName);
      engine.addDefaultKey(configName, configName);

      VERIFY(engine.enterKey("targets"));

      for(const List<String>::Node* i = inputTargets.getFirst(); i; i = i->getNext())
      {
        Map<String, Project>::Node* node = projects.find(i->data);
        Project* project;
        if(node)
          project = &node->data;
        else
        {
          project = &projects.append(i->data, Project(i->data, createSomethingLikeGUID(i->data)));
          String filterName = engine.getFirstKey("folder", false);
          if(!filterName.isEmpty())
          {
            Map<String, ProjectFilter>::Node* node = projectFilters.find(filterName);
            ProjectFilter& filter = node ? node->data : projectFilters.append(filterName, ProjectFilter(createSomethingLikeGUID(filterName)));
            filter.projects.append(project);
          }
        }
        Project::Config& projectConfig = project->configs.append(configKey, Project::Config(config.name, config.platform));

        if(!engine.enterKey(i->data))
        {
          engine.error(String().format(256, "cannot find target \"%s\"", i->data.getData()));
          return false;
        }
        engine.addDefaultKey("target", i->data);
        engine.getKeys("buildCommand", projectConfig.buildCommand, false);
        engine.getKeys("reBuildCommand", projectConfig.reBuildCommand, false);
        engine.getKeys("cleanCommand", projectConfig.cleanCommand, false);
        engine.getKeys("buildCommand", projectConfig.buildCommand, false);
        engine.getKeys("preLinkCommand", projectConfig.preLinkCommand, false);
        engine.getKeys("cleanCommand", projectConfig.cleanCommand, false);
        engine.getKeys("postBuildCommand", projectConfig.postBuildCommand, false);
        projectConfig.buildDir = engine.getFirstKey("buildDir", true);
        projectConfig.firstCommand = engine.getFirstKey("command", false);
        engine.getKeys("cppFlags", projectConfig.cppFlags, true);
        List<String> linkFlags;
        engine.getKeys("linkFlags", linkFlags, true);
        for(const List<String>::Node* i = linkFlags.getFirst(); i; i = i->getNext())
          projectConfig.linkFlags.append(i->data, 0);
        projectConfig.firstOutput = engine.getFirstKey("outputs", false);
        engine.getKeys("outputs", projectConfig.outputs, false);
        engine.getKeys("defines", projectConfig.defines, true);
        engine.getKeys("includePaths", projectConfig.includePaths, true);
        engine.getKeys("libPaths", projectConfig.libPaths, true);
        engine.getKeys("libs", projectConfig.libs, true);
        List<String> dependencies;
        engine.getKeys("dependencies", dependencies, false);
        for(const List<String>::Node* i = dependencies.getFirst(); i; i = i->getNext())
          if(!project->dependencies.find(i->data))
            project->dependencies.append(i->data);
        List<String> root;
        engine.getKeys("root", root, true);
        for(const List<String>::Node* i = root.getFirst(); i; i = i->getNext())
          project->roots.append(i->data);
        if(engine.enterKey("files"))
        {
          List<String> files;
          engine.getKeys(files);
          for(const List<String>::Node* i = files.getFirst(); i; i = i->getNext())
          {
            Map<String, Project::File>::Node* node = project->files.find(i->data);
            Project::File& file = node ? node->data : project->files.append(i->data);
            Project::File::Config& fileConfig = file.configs.append(configKey);
            if(engine.enterKey(i->data))
            {
              engine.addDefaultKey("file", i->data);
              engine.getKeys("command", fileConfig.command, false);
              file.filter = engine.getFirstKey("folder", false);
              String firstCommand = fileConfig.command.isEmpty() ? String() : fileConfig.command.getFirst()->data;
              String type;
              if(firstCommand == "__clCompile")
                type = "ClCompile";
              else if(firstCommand == "__rcCompile")
                type = "ResourceCompile";
              else if(!firstCommand.isEmpty())
              {
                type = "CustomBuild";
                engine.getKeys("message", fileConfig.message, false);
                engine.getKeys("outputs", fileConfig.outputs, false);
                engine.getKeys("inputs", fileConfig.inputs, false);
              }
              if(!type.isEmpty())
              {
                if(!file.type.isEmpty() && file.type != type)
                {
                  // TODO: warning or error?
                }
                else
                  file.type = type;
              }
              engine.getKeys("dependencies", fileConfig.dependencies, false);
              for(const List<String>::Node* i = fileConfig.dependencies.getFirst(); i; i = i->getNext())
                if(!project->dependencies.find(i->data))
                  project->dependencies.append(i->data);
              engine.leaveKey();
            }
          }
          for(Map<String, Project::File>::Node* i = project->files.getFirst(); i; i = i->getNext())
            if(i->data.type.isEmpty())
            {
              String extension = File::getExtension(i->key);
              if(extension == "h" || extension == "hh" || extension == "hxx"  || extension == "hpp")
                i->data.type = "ClInclude";
              else
                i->data.type = "None";
            }

          engine.leaveKey();
        }
        engine.leaveKey();
      }
      engine.leaveKey();
      engine.leaveKey();
    }

    engine.leaveKey();
    engine.leaveKey();
  }

  return true;
}

bool Vcxproj::generateSln()
{
  // find "active" projects (first project and all its dependencies)
  Map<String, void*> buildProjects;
  if(!projects.isEmpty())
  {
    struct A
    {
      static void addProjectDeps(const Project& project, Vcxproj& vcxproj, Map<String, void*>& buildProjects)
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

  // create solution file name
  if(solutionName.isEmpty() && !projects.isEmpty())
    solutionName = projects.getFirst()->data.name;

  // open output file
  fileOpen(solutionName + ".sln");

  // header
  fileWrite("﻿\r\n");
  fileWrite("Microsoft Visual Studio Solution File, Format Version 11.00\r\n");
  fileWrite("# Visual Studio 2010\r\n");

  // project list
  for(const Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    fileWrite(String("Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"") + i->key + "\", \"" + i->key + ".vcxproj\", \"{" + i->data.guid + "}\"\r\n");
    fileWrite("EndProject\r\n");
  }

  // project filter list
  for(const Map<String, ProjectFilter>::Node* i = projectFilters.getFirst(); i; i = i->getNext())
  {
    fileWrite(String("Project(\"{2150E333-8FDC-42A3-9474-1A3956D46DE8}\") = \"") + i->key + "\", \"" + i->key + "\", \"{" + i->data.guid + "}\"\r\n");
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
    }
    fileWrite("\tEndGlobalSection\r\n");
  }

  //
  fileWrite("EndGlobal\r\n");

  fileClose();
  return true;
}

bool Vcxproj::generateVcxprojs()
{
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
    if(!generateVcxproj(i->data))
      return false;
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
    if(!generateVcxprojFilter(i->data))
      return false;
  return true;
}

bool Vcxproj::generateVcxproj(Project& project)
{
  fileOpen(project.name + ".vcxproj");

  fileWrite("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n");
  fileWrite("<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\r\n");
  
  fileWrite("  <ItemGroup Label=\"ProjectConfigurations\">\r\n");
  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    const Project::Config& config = i->data;
    fileWrite(String("    <ProjectConfiguration Include=\"") + i->key + "\">\r\n");
    fileWrite(String("      <Configuration>") + config.name + "</Configuration>\r\n");
    fileWrite(String("      <Platform>") + config.platform + "</Platform>\r\n");
    fileWrite("    </ProjectConfiguration>\r\n");
  }
  fileWrite("  </ItemGroup>\r\n");
  
  fileWrite("  <PropertyGroup Label=\"Globals\">\r\n");
  //fileWrite(String("    <ProjectName>") + project.name + "</ProjectName>\r\n");
  fileWrite(String("    <ProjectGuid>{") + project.guid + "}</ProjectGuid>\r\n");
  fileWrite(String("    <RootNamespace>") + project.name + "</RootNamespace>\r\n");
  fileWrite("  </PropertyGroup>\r\n");

  fileWrite("  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\r\n");

  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    const Project::Config& config = i->data;
    fileWrite(String("  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\" Label=\"Configuration\">\r\n");
    String configurationType("Utility");
    if(config.firstCommand == "__Application" || config.firstCommand == "__StaticLibrary" || 
      config.firstCommand == "__DynamicLibrary" || config.firstCommand == "__Makefile")
      configurationType = config.firstCommand.substr(2);
    fileWrite(String("    <ConfigurationType>") + configurationType + "</ConfigurationType>\r\n");

    if(config.name == "Debug") 
      fileWrite("    <UseDebugLibraries>true</UseDebugLibraries>\r\n"); // i have no idea what this option does and how to change it in the project settings in visual studio
    else                                                                // appearantly it changes some compiler/linker default values?
      fileWrite("    <UseDebugLibraries>false</UseDebugLibraries>\r\n");
    if(config.linkFlags.find("/LTCG"))
      fileWrite("    <WholeProgramOptimization>true</WholeProgramOptimization>\r\n");
    
    fileWrite("  </PropertyGroup>\r\n");
  }

  fileWrite("  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\r\n");
  fileWrite("  <ImportGroup Label=\"ExtensionSettings\">\r\n");
  fileWrite("  </ImportGroup>\r\n");
  
  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    //const Project::Config& config = i->data;
    fileWrite(String("  <ImportGroup Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\" Label=\"PropertySheets\">\r\n");
    fileWrite("    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\r\n");
    fileWrite("  </ImportGroup>\r\n");
  }
  fileWrite("  <PropertyGroup Label=\"UserMacros\" />\r\n");

  fileWrite("  <PropertyGroup>\r\n");
  //fileWrite("    <_ProjectFileVersion>10.0.30319.1</_ProjectFileVersion>\r\n");
  
  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    const Project::Config& config = i->data;

    if(!config.firstOutput.isEmpty())
    {
      String path = File::getDirname(config.firstOutput);
      path.subst("/", "\\");
      fileWrite(String("    <OutDir Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + path + "\\</OutDir>\r\n");
    }
    if(!config.buildDir.isEmpty())
    {
      String path = config.buildDir;
      path.subst("/", "\\");
      if(config.firstOutput.isEmpty())
        fileWrite(String("    <OutDir Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + path + "\\</OutDir>\r\n");
      fileWrite(String("    <IntDir Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + path + "\\</IntDir>\r\n");
    }
    if(config.firstCommand == "__Makefile")
    {
      fileWrite(String("    <NMakeBuildCommandLine Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + joinCommands(config.buildCommand) + "</NMakeBuildCommandLine>\r\n");
      fileWrite(String("    <NMakeReBuildCommandLine Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + joinCommands(config.reBuildCommand) + "</NMakeReBuildCommandLine>\r\n");
      fileWrite(String("    <NMakeCleanCommandLine Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + joinCommands(config.cleanCommand) + "</NMakeCleanCommandLine>\r\n");
      if(!config.outputs.isEmpty())
        fileWrite(String("    <NMakeOutput Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + join(config.outputs) + "</NMakeOutput>\r\n");
      /*
      fileWrite(String("    <NMakePreprocessorDefinitions Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\"><?lua=formatTable(config.defines, \";\")?>;$(NMakePreprocessorDefinitions)</NMakePreprocessorDefinitions>\r\n");
      fileWrite(String("    <NMakeIncludeSearchPath Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\"><?lua=formatTable(config.includePaths, \";\", nil, nil, xmlEscapePath)?>;$(NMakeIncludeSearchPath)</NMakeIncludeSearchPath>\r\n");
      fileWrite(String("    <NMakeForcedIncludes Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">$(NMakeForcedIncludes)</NMakeForcedIncludes>\r\n");
      fileWrite(String("    <NMakeAssemblySearchPath Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">$(NMakeAssemblySearchPath)</NMakeAssemblySearchPath>\r\n");
      fileWrite(String("    <NMakeForcedUsingAssemblies Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">$(NMakeForcedUsingAssemblies)</NMakeForcedUsingAssemblies>\r\n");
      */
    }
    else
    {
      if(config.linkFlags.find("/INCREMENTAL"))
        fileWrite(String("    <LinkIncremental Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">true</LinkIncremental>\r\n");
      //else
        //fileWrite(String("    <LinkIncremental Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\" />\r\n");
    }
  }

  /*
  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    //const Project::Config& config = i->data;
    fileWrite(String("    <CodeAnalysisRuleSet Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">AllRules.ruleset</CodeAnalysisRuleSet>\r\n");
    fileWrite(String("    <CodeAnalysisRules Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\" />\r\n");
    fileWrite(String("    <CodeAnalysisRuleAssemblies Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\" />\r\n");
  }
  */

  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    const Project::Config& config = i->data;
    if(!config.firstOutput.isEmpty())
    {
      String basename = File::getBasename(config.firstOutput);
      fileWrite(String("    <TargetName Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + File::getWithoutExtension(basename) + "</TargetName>\r\n");
      fileWrite(String("    <TargetExt Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">." + File::getExtension(basename) + "</TargetExt>\r\n");
    }
  }

  fileWrite("  </PropertyGroup>\r\n");
  
  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    const Project::Config& config = i->data;
    fileWrite(String("  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">\r\n");
    
    if(!config.postBuildCommand.isEmpty())
    {
      fileWrite("    <PreBuildEvent>\r\n");
      fileWrite(String("      <Command>") + joinCommands(config.postBuildCommand) + "</Command>\r\n");
      fileWrite("    </PreBuildEvent>\r\n");
    }
    if(!config.preLinkCommand.isEmpty())
    {
      fileWrite("    <PreLinkEvent>\r\n");
      fileWrite(String("      <Command>") + joinCommands(config.preLinkCommand) + "</Command>\r\n");
      fileWrite("    </PreLinkEvent>\r\n");
    }
    if(!config.postBuildCommand.isEmpty())
    {
      fileWrite("    <PostBuildEvent>\r\n");
      fileWrite(String("      <Command>") + joinCommands(config.postBuildCommand) + "</Command>\r\n");
      fileWrite("    </PostBuildEvent>\r\n");
    }

    if(config.firstCommand != "__Makefile")
    {/*
      fileWrite("    <Midl>\r\n");
      fileWrite("      <WarningLevel>0</WarningLevel>\r\n");
      fileWrite("      <DefaultCharType>Unsigned</DefaultCharType>\r\n");
      fileWrite("      <EnableErrorChecks>None</EnableErrorChecks>\r\n");
      fileWrite("    </Midl>\r\n");
      */
      fileWrite("    <ClCompile>\r\n");

      {
        List<String> additionalOptions;
        for(const List<String>::Node* i = config.cppFlags.getFirst(); i; i = i->getNext())
          if(!knownCppOptions.find(i->data))
            additionalOptions.append(i->data);
      
        if(!additionalOptions.isEmpty())
          fileWrite(String("      <AdditionalOptions>") + join(additionalOptions, ' ') + " %(AdditionalOptions)</AdditionalOptions>\r\n");

        Map<String, void*> usedOptions;
        for(const List<String>::Node* i = config.cppFlags.getFirst(); i; i = i->getNext())
        {
          const Map<String, Option>::Node* node = knownCppOptions.find(i->data);
          if(node)
          {
            const Option& option = node->data;
            if(usedOptions.find(option.name))
            {
              // TODO: warning or error
            }
            else
            {
              usedOptions.append(option.name);
              fileWrite(String("      <") + option.name + ">" + option.value + "</" + option.name + ">\r\n");
            }
          }
        }
      }
      
      if(!config.includePaths.isEmpty())
        fileWrite(String("      <AdditionalIncludeDirectories>") + join(config.includePaths) + ";%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\r\n");
      if(!config.defines.isEmpty())
        fileWrite(String("      <PreprocessorDefinitions>") + join(config.defines) + ";%(PreprocessorDefinitions)</PreprocessorDefinitions>\r\n");
      fileWrite("    </ClCompile>\r\n");
      
      fileWrite("    <ResourceCompile>\r\n");
      if(!config.defines.isEmpty())
        fileWrite(String("      <PreprocessorDefinitions>") + join(config.defines) + ";%(PreprocessorDefinitions)</PreprocessorDefinitions>\r\n");
      fileWrite("    </ResourceCompile>\r\n");
      
      fileWrite("    <ProjectReference>\r\n");
      //fileWrite("      <UseLibraryDependencyInputs>false</UseLibraryDependencyInputs>\r\n");
      fileWrite("      <LinkLibraryDependencies>false</LinkLibraryDependencies>\r\n");
      fileWrite("    </ProjectReference>\r\n");
      
      fileWrite("    <Link>\r\n");

      {
        List<String> additionalOptions;
        for(const Map<String, void*>::Node* i = config.linkFlags.getFirst(); i; i = i->getNext())
          if(!knownLinkOptions.find(i->key))
            additionalOptions.append(i->key);
      
        if(!additionalOptions.isEmpty())
          fileWrite(String("      <AdditionalOptions>") + join(additionalOptions, ' ') + " %(AdditionalOptions)</AdditionalOptions>\r\n");

        Map<String, void*> usedOptions;
        for(const Map<String, void*>::Node* i = config.linkFlags.getFirst(); i; i = i->getNext())
        {
          const Map<String, Option>::Node* node = knownLinkOptions.find(i->key);
          if(node)
          {
            const Option& option = node->data;
              if(option.name.isEmpty())
              continue;
            if(usedOptions.find(option.name))
            {
              // TODO: warning or error
            }
            else
            {
              usedOptions.append(option.name);
              fileWrite(String("      <") + option.name + ">" + option.value + "</" + option.name + ">\r\n");
            }
          }
        }
      }

      if(!config.libs.isEmpty())
        fileWrite(String("      <AdditionalDependencies>") + join(config.libs, ';', ".lib") + ";%(AdditionalDependencies)</AdditionalDependencies>\r\n");
      if(!config.libPaths.isEmpty())
        fileWrite(String("      <AdditionalLibraryDirectories>") + join(config.libPaths) + ";%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>\r\n");

      fileWrite("    </Link>\r\n");
    }
    fileWrite("  </ItemDefinitionGroup>\r\n");
  }

  fileWrite("  <ItemGroup>\r\n");
  for(Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
  {
    Project::File& file = i->data;
    String path = i->key;
    path.subst("/", "\\");

    if(file.type == "CustomBuild")
    {
      fileWrite(String("    <CustomBuild Include=\"") + path + "\">\r\n");
      const String& fileName = i->key;
      for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
      {
        const String& configKey = i->key;
        Map<String, Project::File::Config>::Node* node = file.configs.find(i->key);
        if(node)
        {
          Project::File::Config& config = node->data;
          if(!config.message.isEmpty())
            fileWrite(String("      <Message Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + join(config.message, ' ') + "</Message>\r\n");
          if(!config.command.isEmpty())
            fileWrite(String("      <Command Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + joinCommands(config.command) + "</Command>\r\n");
          else
            {
            // TODO: warning or error
          }

          // resolve target dependencies (add outputs of the target to the list of input files)
          {
            for(const List<String>::Node* i = config.dependencies.getFirst(); i; i = i->getNext())
            {
              const Map<String, Project>::Node* node = projects.find(i->data);
              if(node)
              {
                const Project& depProj = node->data;
                const Map<String, Project::Config>::Node* node = depProj.configs.find(configKey);
                if(node)
                  for(const List<String>::Node* i = node->data.outputs.getFirst(); i; i = i->getNext())
                    config.inputs.append(i->data);
              }
            }
          }

          List<String> additionalInputs;
          {
            for(const List<String>::Node* i = config.inputs.getFirst(); i; i = i->getNext())
              if(i->data != fileName)
                additionalInputs.append(i->data);
          }

          if(!additionalInputs.isEmpty())
            fileWrite(String("      <AdditionalInputs Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + join(additionalInputs) + ";%(AdditionalInputs)</AdditionalInputs>\r\n");

          if(!config.outputs.isEmpty())
            fileWrite(String("      <Outputs Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + join(config.outputs) + ";%(Outputs)</Outputs>\r\n");
          else
          {
            // TODO: warning or error
          }
        }
        else
          fileWrite(String("      <ExcludedFromBuild Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">true</ExcludedFromBuild>\r\n");
      }
      fileWrite("    </CustomBuild>\r\n");
    }
    else
    {
      if(file.configs.getSize() < project.configs.getSize())
      {
        fileWrite(String("    <") + file.type + " Include=\"" + path + "\">\r\n");
        for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
          if(!file.configs.find(i->key))
            fileWrite(String("      <ExcludedFromBuild Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">true</ExcludedFromBuild>\r\n");
        fileWrite(String("    </") + file.type + ">\r\n");
      }
      else
        fileWrite(String("    <") + file.type + " Include=\"" + path + "\" />\r\n");
    }
  }
  fileWrite("  </ItemGroup>\r\n");

  if(!project.dependencies.isEmpty())
  {
    fileWrite("  <ItemGroup>\r\n");
    for(const Map<String, void*>::Node* i = project.dependencies.getFirst(); i; i = i->getNext())
    {
      const Map<String, Project>::Node* node = projects.find(i->key);
      if(node)
      {
        const Project& project = node->data;
        fileWrite(String("    <ProjectReference Include=\"") + project.name + ".vcxproj\">\r\n");
        fileWrite(String("      <Project>{") + project.guid + "}</Project>\r\n");
        //fileWrite("      <ReferenceOutputAssembly>false</ReferenceOutputAssembly>\r\n");
        fileWrite("    </ProjectReference>\r\n");
      }
    }
    fileWrite("  </ItemGroup>\r\n");
  }
  
  fileWrite("  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\r\n");
  fileWrite("  <ImportGroup Label=\"ExtensionTargets\">\r\n");
  fileWrite("  </ImportGroup>\r\n");
  fileWrite("</Project>\r\n");

  fileClose();
  return true;
}

bool Vcxproj::generateVcxprojFilter(Project& project)
{
  fileOpen(project.name + ".vcxproj.filters");
  fileWrite("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n");
  fileWrite("<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\r\n");

  Map<String, String> filters;
  for(Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
  {
    Project::File& file = i->data;

    if(!file.filter.isEmpty()) // a user defined filter
    {
      if(!filters.find(file.filter))
        filters.append(file.filter);
      continue;
    }

    // create a filter based on the file system hierarchy
    List<String> filtersToAdd;
    String root;
    String filterName = File::getDirname(i->key);
    for(;;)
    {
      if(filterName == "." || File::getBasename(filterName) == "..")
        break;
      const Map<String, void*>::Node* node = project.roots.find(filterName);
      if(node)
      {
        root = node->key;
        break;
      }
      filtersToAdd.prepend(filterName);
      filterName = File::getDirname(filterName);
    }
    for(const List<String>::Node* i = filtersToAdd.getFirst(); i; i = i->getNext())
    {
      String filterName = root.isEmpty() ? i->data : i->data.substr(root.getLength() + 1);
      if(root.isEmpty())
      { // remove leading ../ and ./
        for(;;)
        {
          if(strncmp(filterName.getData(), "../", 3) == 0)
            filterName = filterName.substr(3);
          else if(strncmp(filterName.getData(), "./", 2) == 0)
            filterName = filterName.substr(2);
          else
            break;
        }
      }
      filterName.subst("/", "\\");
      if(!filters.find(filterName))
        filters.append(filterName, createSomethingLikeGUID(filterName));
      if(i == filtersToAdd.getLast())
        file.filter = filterName;
    }
  }


  fileWrite("  <ItemGroup>\r\n");
  for(const Map<String, String>::Node* i = filters.getFirst(); i; i = i->getNext())
  {
    fileWrite(String("    <Filter Include=\"") + i->key + "\">\r\n");
    fileWrite(String("      <UniqueIdentifier>{") + i->data + "}</UniqueIdentifier>\r\n");
    fileWrite("    </Filter>\r\n");
  }
  fileWrite("  </ItemGroup>\r\n");
  
  
  fileWrite("  <ItemGroup>\r\n");
  for(const Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
  {
    const Project::File& file = i->data;
    String path = i->key;
    path.subst("/", "\\");
    if(file.filter.isEmpty())
      fileWrite(String("    <") + file.type + " Include=\"" + path + "\" />\r\n");
    else
    {
      fileWrite(String("    <") + file.type + " Include=\"" + path + "\">\r\n");
      fileWrite(String("      <Filter>") + file.filter + "</Filter>\r\n");
      fileWrite(String("    </") + file.type + ">\r\n");
    }
  }
  
  fileWrite("  </ItemGroup>\r\n");
  
  fileWrite("</Project>\r\n");

  fileClose();
  return true;
}

void Vcxproj::fileOpen(const String& name)
{
  if(!file.open(name, File::writeFlag))
  {
    engine.error(file.getErrno().getString());
    exit(EXIT_FAILURE);
  }
}

void Vcxproj::fileWrite(const String& data)
{
  if(!file.write(data))
  {
    engine.error(file.getErrno().getString());
    exit(EXIT_FAILURE);
  }
}

String Vcxproj::createSomethingLikeGUID(const String& name)
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

String Vcxproj::join(const List<String>& items, char sep, const String& suffix) const
{
  String result;
  const List<String>::Node* i = items.getFirst();
  if(i)
  {
    result = i->data;
    result.append(suffix);
    for(i = i->getNext(); i; i = i->getNext())
    {
      result.append(sep);
      result.append(i->data);
      result.append(suffix);
    }
  }
  return result;
}

String Vcxproj::joinCommands(const List<String>& commands) const
{
  List<String> commands2;
  commands2 = commands;
  if(!commands2.isEmpty())
    commands2.getFirst()->data.subst("/", "\\");
  String result;
  Words::append(commands2, result);
  return result;
  // TODO: something for more than a single command?
}
