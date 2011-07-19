
//#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "Tools/Words.h"
#include "Tools/File.h"
#include "Tools/md5.h"
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
  engine.addDefaultKey("vcxproj", "true"); // temp
  engine.addDefaultKey("host", "Win32");
  engine.addDefaultKey("platforms", "Win32");
  engine.enterDefaultKey("configurations");
    engine.addResolvableKey("Debug");
    engine.addResolvableKey("Release");
  engine.leaveKey();
  engine.addDefaultKey("targets");
  engine.addDefaultKey("buildDir", "$(configuration)");
  engine.addDefaultKey("cppFlags", "$(if $(Debug),,/O2 /Oy)");
  engine.addDefaultKey("linkFlags", "$(if $(Debug),/INCREMENTAL /DEBUG,/OPT:REF /OPT:ICF)");
  engine.enterDefaultKey("cppSource");
    engine.addResolvableKey("command", "__clCompile");
  engine.leaveKey();
  engine.enterDefaultKey("cSource");
    engine.addResolvableKey("command", "__clCompile");
  engine.leaveKey();
  engine.enterDefaultKey("rcSource");
    engine.addResolvableKey("command", "__rcCompile");
  engine.leaveKey();
  engine.enterDefaultKey("cppApplication");
    engine.addResolvableKey("command", "__Application");
    engine.addResolvableKey("outputs", "$(buildDir)/$(target).exe");
  engine.leaveKey();
  engine.enterDefaultKey("cppDynamicLibrary");
    engine.addResolvableKey("command", "__DynamicLibrary");
    engine.addResolvableKey("outputs", "$(buildDir)/$(patsubst lib%,%,$(target)).dll");
  engine.leaveKey();
  engine.enterDefaultKey("cppStaticLibrary");
    engine.addResolvableKey("command", "__StaticLibrary");
    engine.addResolvableKey("outputs", "$(buildDir)/$(patsubst lib%,%,$(target)).lib");
  engine.leaveKey();
  engine.enterDefaultKey("cApplication");
    engine.addResolvableKey("command", "__Application");
    engine.addResolvableKey("outputs", "$(buildDir)/$(target).exe");
  engine.leaveKey();
  engine.enterDefaultKey("cDynamicLibrary");
    engine.addResolvableKey("command", "__DynamicLibrary");
    engine.addResolvableKey("outputs", "$(buildDir)/$(patsubst lib%,%,$(target)).dll");
  engine.leaveKey();
  engine.enterDefaultKey("cStaticLibrary");
    engine.addResolvableKey("command", "__StaticLibrary");
    engine.addResolvableKey("outputs", "$(buildDir)/$(patsubst lib%,%,$(target)).lib");
  engine.leaveKey();

  // add user arguments
  engine.enterUnnamedKey();
  for(const Map<String, String>::Node* i = userArgs.getFirst(); i; i = i->getNext())
    engine.addDefaultKey(i->key, i->data);

  // get some global keys
  solutionName = engine.getFirstKey("name");
  List<String> inputPlatforms;
  engine.getKeys("platforms", inputPlatforms);
  List<String> target;
  engine.getKeys("target", target);
  for(const List<String>::Node* i = target.getFirst(); i; i = i->getNext())
    activesProjects.append(i->data, 0);

  for(const List<String>::Node* i = inputPlatforms.getFirst(); i; i = i->getNext())
  {
    const String& platform = i->data;
    engine.enterUnnamedKey();
    engine.addDefaultKey("platform", platform);
    engine.addDefaultKey(platform, "true"); // temp

    // enter configurations space
    engine.enterKey("configurations");

    // get configuration project list
    List<String> configurations;
    engine.getKeys(configurations);
    for(const List<String>::Node* i = configurations.getFirst(); i; i = i->getNext())
    {
      const String& configName = i->data;
      const String configKey = configName + "|" + platform;
      const Config& config = configs.append(configKey, Config(configName, platform));

      engine.enterKey(configName);
      engine.addDefaultKey("configuration", configName);
      engine.addDefaultKey(configName, "true"); // temp

      List<String> targets;
      engine.enterKey("targets");
      engine.getKeys(targets);
      for(const List<String>::Node* i = targets.getFirst(); i; i = i->getNext())
      {
        Map<String, Project>::Node* node = projects.find(i->data);
        Project* project;
        if(node)
          project = &node->data;
        else
        {
          project = &projects.append(i->data, Project(i->data, createSomethingLikeGUID(i->data)));
          String filterName = engine.getFirstKey("filter");
          if(!filterName.isEmpty())
          {
            Map<String, ProjectFilter>::Node* node = projectFilters.find(filterName);
            ProjectFilter& filter = node ? node->data : projectFilters.append(filterName, ProjectFilter(createSomethingLikeGUID(filterName)));
            filter.projects.append(project);
          }
        }
        Project::Config& projectConfig = project->configs.append(configKey, Project::Config(config.name, config.platform));

        engine.enterKey(i->data);
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
              engine.getKeys("command", fileConfig.command);
              String firstCommand = fileConfig.command.isEmpty() ? String() : fileConfig.command.getFirst()->data;
              String type;
              if(firstCommand == "__clCompile" || firstCommand == "__rcCompile")
                type = firstCommand;
              else if(!firstCommand.isEmpty())
                type = "__CustomBuildTool";
              else
                type = file.type;
              if(!file.type.isEmpty() && file.type != type)
              {
                // TODO: warning or error?
              }
              else
                file.type = type;
              engine.leaveKey();
            }
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
  if(solutionName.isEmpty() && !projects.isEmpty())
    solutionName = projects.getFirst()->data.name;

  // generate solution file
  if(!generateSln())
    return false;

  // generate project files
  if(!generateVcxprojs())
    return false;

  return true;
}

bool Vcxproj::generateSln()
{
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
      if(activesProjects.isEmpty() || activesProjects.find(project.name))
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
      /*
      fileWrite("    <ProjectReference>\r\n");
      fileWrite("      <UseLibraryDependencyInputs>true</UseLibraryDependencyInputs>\r\n");
      fileWrite("    </ProjectReference>\r\n");
      */
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
  for(const Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
  {
    const Project::File& file = i->data;
    String path = i->key;
    path.subst("/", "\\");

    if(file.type == "__CustomBuildTool")
    {
      /*
      fileWrite(String("    <CustomBuild Include=\"") + path + "\">\r\n");
      fileWrite("<?lua for i, config in ipairs(configs) do local include, flag = includes[i]; if flags then flag = flags[i]; end ?>\r\n");
      fileWrite("<?lua if include ~= nil and flag and flag.target and flag.command then ?>\r\n");
      fileWrite("      <Message Condition=\"'$(Configuration)|$(Platform)'=='<?lua=config.config?>'\"><?lua=xmlEscape(flag.description)?></Message>\r\n");
      fileWrite("      <Command Condition=\"'$(Configuration)|$(Platform)'=='<?lua=config.config?>'\"><?lua if type(flag.command) == \"function\" then echo(formatTable(flag.command(flag, config), \"&#x0D;&#x0A;\", nil, nil, xmlEscape)); else echo(formatTable(flag.command, \"&#x0D;&#x0A;\", nil, nil, xmlEscape)); end ?></Command>\r\n");
      fileWrite("<?lua if type(flag.dependencies) == \"table\" then ?>\r\n");
      fileWrite("      <AdditionalInputs Condition=\"'$(Configuration)|$(Platform)'=='<?lua=config.config?>'\"><?lua=formatTable(flag.dependencies, \";\")?>;%(AdditionalInputs)</AdditionalInputs>\r\n");
      fileWrite("<?lua end ?>\r\n");
      fileWrite("      <Outputs Condition=\"'$(Configuration)|$(Platform)'=='<?lua=config.config?>'\"><?lua=formatTable(flag.target, \";\")?>;%(Outputs)</Outputs>\r\n");
      fileWrite("<?lua else ?>\r\n");
      fileWrite("      <ExcludedFromBuild Condition=\"'$(Configuration)|$(Platform)'=='<?lua=config.config?>'\">true</ExcludedFromBuild>\r\n");
      fileWrite("<?lua end ?>\r\n");
      fileWrite("<?lua end ?>\r\n");
      fileWrite("    </CustomBuild>\r\n");
      */
    }
    else
    {
      String type = "None";
      if(file.type == "__clCompile")
        type = "ClCompile";
      else if(file.type == "__rcCompile")
        type = "ResourceCompile";
      else
      {
        String extension = File::getExtension(i->key);
        if(extension == "h" || extension == "hh" || extension == "hxx"  || extension == "hpp")
          type = "ClInclude";
      }

      if(file.configs.getSize() < project.configs.getSize())
      {
        fileWrite(String("    <") + type + " Include=\"" + path + "\">\r\n");
        for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
          if(!file.configs.find(i->key))
            fileWrite(String("      <ExcludedFromBuild Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">true</ExcludedFromBuild>\r\n");
        fileWrite(String("    </") + type + ">\r\n");
      }
      else
        fileWrite(String("    <") + type + " Include=\"" + path + "\" />\r\n");
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
  String result;
  Words::append(commands, result);
  return result;
  // TODO: something for more than a single command?
}
