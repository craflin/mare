
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>

#include "Tools/Word.h"
#include "Tools/File.h"
#include "Tools/Directory.h"
#include "Tools/md5.h"
#include "Tools/Assert.h"
#include "Tools/Error.h"
#include "Engine.h"

#include "Vcxproj.h"

bool Vcxproj::generate(const Map<String, String>& userArgs)
{

  // C/C++ general tab:
  OptionGroup* group = &knownOptionGroups.append(OptionGroup("DebugInformationFormat", String())); // TODO: unsetValue
  knownCppOptions.append("/Z7", Option(group, "OldStyle"));
  knownCppOptions.append("/Zi", Option(group, "ProgramDatabase"));
  knownCppOptions.append("/ZI", Option(group, "EditAndContinue"));
  // TODO: common language runtime support
  // TODO: suppress startup banner
  group = &knownOptionGroups.append(OptionGroup("WarningLevel", String())); // TODO: unsetValue
  knownCppOptions.append("/W0", Option(group, "TurnOffAllWarnings"));
  knownCppOptions.append("/W1", Option(group, "Level1"));
  knownCppOptions.append("/W2", Option(group, "Level2"));
  knownCppOptions.append("/W3", Option(group, "Level3"));
  knownCppOptions.append("/W4", Option(group, "Level4"));
  knownCppOptions.append("/Wall", Option(group, "EnableAllWarnings"));
  group = &knownOptionGroups.append(OptionGroup("TreatWarningAsError", String())); // TODO: unsetValue
  knownCppOptions.append("/WX", Option(group, "true"));
  knownCppOptions.append("/WX-", Option(group, "false"));
  group = &knownOptionGroups.append(OptionGroup("MultiProcessorCompilation", String())); // TODO: unsetValue
  knownCppOptions.append("/MP", Option(group, "true"));
  // TODO: use unicode for assembler listing

  // C/C++ optimization tab:
  group = &knownOptionGroups.append(OptionGroup("Optimization", String())); // TODO: unsetValue
  knownCppOptions.append("/O1", Option(group, "MinSpace"));
  knownCppOptions.append("/O2", Option(group, "MaxSpeed"));
  knownCppOptions.append("/Od", Option(group, "Disabled"));
  knownCppOptions.append("/Ox", Option(group, "Full"));
  group = &knownOptionGroups.append(OptionGroup("InlineFunctionExpansion", String())); // TODO: unsetValue
  knownCppOptions.append("/Ob2", Option(group, "AnySuitable"));
  knownCppOptions.append("/Ob1", Option(group, "OnlyExplicitInline"));
  knownCppOptions.append("/Ob0", Option(group, "Disabled"));
  group = &knownOptionGroups.append(OptionGroup("IntrinsicFunctions", String())); // TODO: unsetValue
  knownCppOptions.append("/Oi", Option(group, "true"));
  group = &knownOptionGroups.append(OptionGroup("FavorSizeOrSpeed", String())); // TODO: unsetValue
  knownCppOptions.append("/Os", Option(group, "Size"));
  knownCppOptions.append("/Ot", Option(group, "Speed"));
  group = &knownOptionGroups.append(OptionGroup("OmitFramePointers", String())); // TODO: unsetValue
  knownCppOptions.append("/Oy", Option(group, "true"));
  knownCppOptions.append("/Oy-", Option(group, "false"));
  group = &knownOptionGroups.append(OptionGroup("EnableFiberSafeOptimizations", String())); // TODO: unsetValue
  knownCppOptions.append("/GT", Option(group, "true"));
  group = &knownOptionGroups.append(OptionGroup("WholeProgramOptimization", String())); // TODO: unsetValue
  knownCppOptions.append("/GL", Option(group, "true"));

  // TODO: C/C++ Preprocessor tab

  // C/C++ code generation tab:
  group = &knownOptionGroups.append(OptionGroup("StringPooling", String())); // TODO: unsetValue
  knownCppOptions.append("/GF", Option(group, "true"));
  group = &knownOptionGroups.append(OptionGroup("MinimalRebuild", String())); // TODO: unsetValue
  knownCppOptions.append("/Gm", Option(group, "true"));
  knownCppOptions.append("/Gm-", Option(group, "false"));
  // TODO: C++ exceptions
  // TODO: smaller type checks
  // TODO: basic runtime checks
  group = &knownOptionGroups.append(OptionGroup("RuntimeLibrary", String())); // TODO: unsetValue
  knownCppOptions.append("/MT", Option(group, "MultiThreaded"));
  knownCppOptions.append("/MD", Option(group, "MultiThreadedDLL"));
  knownCppOptions.append("/MTd", Option(group, "MultiThreadedDebug"));
  knownCppOptions.append("/MDd", Option(group, "MultiThreadedDebugDLL"));
  group = &knownOptionGroups.append(OptionGroup("BufferSecurityCheck", String())); // TODO: unsetValue
  knownCppOptions.append("/GS", Option(group, "true"));
  knownCppOptions.append("/GS-", Option(group, "false"));
  group = &knownOptionGroups.append(OptionGroup("FunctionLevelLinking", String())); // TODO: unsetValue
  knownCppOptions.append("/Gy", Option(group, "true"));
  knownCppOptions.append("/Gy-", Option(group, "false"));
  group = &knownOptionGroups.append(OptionGroup("EnableEnhancedInstructionSet", String())); // TODO: unsetValue
  knownCppOptions.append("/arch:SSE", Option(group, "StreamingSIMDExtensions"));
  knownCppOptions.append("/arch:SSE2", Option(group, "StreamingSIMDExtensions2"));
  group = &knownOptionGroups.append(OptionGroup("FloatingPointModel", String())); // TODO: unsetValue
  knownCppOptions.append("/fp:precise", Option(group, "Precise"));
  knownCppOptions.append("/fp:strict", Option(group, "Strict"));
  knownCppOptions.append("/fp:fast", Option(group, "Fast"));
  // TODO: floating point exceptions
  // TODO: create hotpatchable image

  // C/C++ language tab
  // TODO: disable language extensions
  group = &knownOptionGroups.append(OptionGroup("TreatWChar_tAsBuiltInType", String())); // TODO: unsetValue
  knownCppOptions.append("/Zc:wchar_t", Option(group, "true"));
  knownCppOptions.append("/Zc:wchar_t-", Option(group, "false"));
  // TODO: force conformance in for loop scope
  // TODO: enable rtti
  // TODO: openmp

  // precompiled header tab
  group = &knownOptionGroups.append(OptionGroup("PrecompiledHeader", "NotUsing", "PrecompiledHeaderFile"));
  knownCppOptions.append("/Yu", Option(group, "Use"));
  knownCppOptions.append("/Yc", Option(group, "Create"));
  group = &knownOptionGroups.append(OptionGroup(String(), String(), "PrecompiledHeaderOutputFile")); // TODO: unsetValue?
  knownCppOptions.append("/Fp", Option(group, String()));

  // TODO: C/C++ output files tab?
  group = &knownOptionGroups.append(OptionGroup(String(), String(), "ForcedIncludeFiles"));
  knownCppOptions.append("/FI", Option(group, String()));

  // TODO: C/C++ browse information tab?
  // TODO: C/C++ advanced tab?

  // linker tabs:

  // TODO: linker general tab
  // TODO: linker input tab
  // TODO: linker manifest file tab

  // linker debugging tab:
  group = &knownOptionGroups.append(OptionGroup("GenerateDebugInformation"));
  knownLinkOptions.append("/DEBUG", Option(group, "true"));
  // TODO: more options?

  // linker system tab:
  group = &knownOptionGroups.append(OptionGroup("SubSystem"));
  knownLinkOptions.append("/SUBSYSTEM:CONSOLE", Option(group, "Console"));
  knownLinkOptions.append("/SUBSYSTEM:WINDOWS", Option(group, "Windows"));
  // TODO: more SubSystems
  // TODO: more system tab options?

  // linker optimization tab:
  knownLinkOptions.append("/INCREMENTAL"); // TODO: ????
  knownLinkOptions.append("/LTCG"); // TODO: ???
  group = &knownOptionGroups.append(OptionGroup("OptimizeReferences"));
  knownLinkOptions.append("/OPT:REF", Option(group, "true"));
  knownLinkOptions.append("/OPT:NOREF", Option(group, "false"));
  group = &knownOptionGroups.append(OptionGroup("EnableCOMDATFolding"));
  knownLinkOptions.append("/OPT:ICF", Option(group, "true"));
  knownLinkOptions.append("/OPT:NOICF", Option(group, "false"));
  // TODO: more LTCG options?
  // TODO: more options?

  // TODO: Manifest Tool tabs?

  // pseudo cppFlags with no actual compiler flag representation
  group = &knownOptionGroups.append(OptionGroup("CharacterSet"));
  knownVsOptions.append("/CharacterSet", Option(group, String()));
  group = &knownOptionGroups.append(OptionGroup("PlatformToolset"));
  knownVsOptions.append("/PlatformToolset", Option(group, String()));
  group = &knownOptionGroups.append(OptionGroup("UseDebugLibraries"));
  knownVsOptions.append("/UseDebugLibraries", Option(group, String()));

  //
  engine.addDefaultKey("tool", "vcxproj");
  engine.addDefaultKey("vcxproj", String().format(128, "%d", version));
  engine.addDefaultKey("host", "Win32");
  engine.addDefaultKey("platforms", "Win32");
  engine.addDefaultKey("configurations", "Debug Release");
  engine.addDefaultKey("targets"); // an empty target list exists per default
  engine.addDefaultKey("buildDir", "$(configuration)");
  engine.addDefaultKey("outputDir", "$(buildDir)");
  engine.addDefaultKey("cppFlags", "/W3 $(if $(Debug),/UseDebugLibraries,/O2 /Oy)");
  engine.addDefaultKey("cFlags", "/W3 $(if $(Debug),/UseDebugLibraries,/O2 /Oy)");
  engine.addDefaultKey("linkFlags", "$(if $(Debug),/INCREMENTAL /DEBUG,/OPT:REF /OPT:ICF)");
  {
    Map<String, String> cSource;
    cSource.append("command", "__cSource");
    engine.addDefaultKey("cSource", cSource);
  }
  {
    Map<String, String> cppSource;
    cppSource.append("command", "__cppSource");
    engine.addDefaultKey("cppSource", cppSource);
  }
  {
    Map<String, String> rcSource;
    rcSource.append("command", "__rcSource");
    engine.addDefaultKey("rcSource", rcSource);
  }
  {
    Map<String, String> cApplication;
    cApplication.append("command", "__cApplication");
    cApplication.append("output", "$(outputDir)/$(target).exe");
    engine.addDefaultKey("cApplication", cApplication);
  }
  {
    Map<String, String> cppApplication;
    cppApplication.append("command", "__cppApplication");
    cppApplication.append("output", "$(outputDir)/$(target).exe");
    engine.addDefaultKey("cppApplication", cppApplication);
  }
  {
    Map<String, String> cDynamicLibrary;
    cDynamicLibrary.append("command", "__cDynamicLibrary");
    cDynamicLibrary.append("output", "$(outputDir)/$(patsubst lib%,%,$(target)).dll");
    engine.addDefaultKey("cDynamicLibrary", cDynamicLibrary);
  }
  {
    Map<String, String> cppDynamicLibrary;
    cppDynamicLibrary.append("command", "__cppDynamicLibrary");
    cppDynamicLibrary.append("output", "$(outputDir)/$(patsubst lib%,%,$(target)).dll");
    engine.addDefaultKey("cppDynamicLibrary", cppDynamicLibrary);
  }
  {
    Map<String, String> cStaticLibrary;
    cStaticLibrary.append("command", "__cStaticLibrary");
    cStaticLibrary.append("output", "$(outputDir)/$(patsubst lib%,%,$(target)).lib");
    engine.addDefaultKey("cStaticLibrary", cStaticLibrary);
  }
  {
    Map<String, String> cppStaticLibrary;
    cppStaticLibrary.append("command", "__cppStaticLibrary");
    cppStaticLibrary.append("output", "$(outputDir)/$(patsubst lib%,%,$(target)).lib");
    engine.addDefaultKey("cppStaticLibrary", cppStaticLibrary);
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
  if(!generateVcxprojs())
    return false;

  return true;
}

bool Vcxproj::readFile()
{
  // get some global keys
  engine.enterRootKey();
  solutionName = engine.getFirstKey("name");
  solutionFile = engine.getFirstKey("solutionFile");
  List<String> allPlatforms, allConfigurations, allTargets;
  engine.getKeys("platforms", allPlatforms);
  engine.getKeys("configurations", allConfigurations);
  engine.getKeys("targets", allTargets);
  engine.leaveKey();

  // do something for each target in each configuration
  for(const List<String>::Node* i = allPlatforms.getFirst(); i; i = i->getNext())
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
        engine.addDefaultKey("mareDir", engine.getMareDir());

        Map<String, Project>::Node* node = projects.find(i->data);
        Project& project = node ? node->data : projects.append(i->data, Project(i->data));
        Project::Config& projectConfig = project.configs.append(configKey, Project::Config(config.name, config.platform));

        if(project.displayName.isEmpty())
          project.displayName = engine.getFirstKey("name", false);
        if(project.filter.isEmpty())
          project.filter = engine.getFirstKey("folder", false);
        if(project.projectFile.isEmpty())
          project.projectFile = engine.getFirstKey("projectFile", false);

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

        engine.getKeys("cppFlags", projectConfig.cppFlags, true);
        engine.getKeys("cFlags", projectConfig.cFlags, true);
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
            fileConfig.hasCppFlags = engine.getKeys("cppFlags", fileConfig.cppFlags, false);
            fileConfig.hasCFlags = engine.getKeys("cFlags", fileConfig.cFlags, false);
            engine.leaveKey(); // VERIFY(engine.enterKey(i->data));
            engine.leaveKey();
          }

          engine.leaveKey();
        }

        List<String> visualizers;
        engine.getKeys("visualizers", visualizers, true);
        for(const List<String>::Node* i = visualizers.getFirst(); i; i = i->getNext())
        {
          bool contained = false;
          for(const List<String>::Node* j = project.visualizers.getFirst(); j; j = j->getNext())
            if(i->data == j->data)
            {
              contained = true;
              break;
            }
          if(!contained)
            project.visualizers.append(i->data);
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

bool Vcxproj::processData()
{
  // prepare project and file types
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    Project& project = i->data;

    // get display name and guid
    if(project.displayName.isEmpty())
      project.displayName = i->key;
    project.guid = createSomethingLikeGUID(i->key + project.displayName);

    // project dir?
    if(!project.projectFile.isEmpty())
    {
      project.projectDir = File::getDirname(project.projectFile);
      if(project.projectDir == ".")
        project.projectDir = String();
    }
    else
      project.projectFile = project.name + ".vcxproj";

    // handle project filter (solution explorer folder)
    if(!project.filter.isEmpty())
    {
      List<String> filtersToAdd;
      String filterName = File::simplifyPath(project.filter);
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

      Map<String, void*> defines;
      for(List<String>::Node* i = projectConfig.defines.getFirst(); i; i = i->getNext())
        defines.append(i->data, 0);
      if(defines.find("UNICODE") || defines.find("_UNICODE"))
      {
        if(!projectConfig.vsOptions.find("CharacterSet"))
          projectConfig.vsOptions.append("CharacterSet", "Unicode");
      }
      else if(defines.find("_MBCS"))
      {
        if(!projectConfig.vsOptions.find("CharacterSet"))
          projectConfig.vsOptions.append("CharacterSet", "MultiByte");
      }
      if(defines.find("_DEBUG"))
      {
        if(!projectConfig.vsOptions.find("UseDebugLibraries"))
          projectConfig.vsOptions.append("UseDebugLibraries", "true");
      }

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
        else if(firstCommandWord == "__cppApplication" || firstCommandWord == "__cppStaticLibrary" || firstCommandWord == "__cppDynamicLibrary")
        {
          projectConfig.type = firstCommandWord.substr(5);
          projectConfig.command.clear();
        }
        else if(firstCommandWord == "__cApplication" || firstCommandWord == "__cStaticLibrary" || firstCommandWord == "__cDynamicLibrary")
        {
          projectConfig.language = Project::Config::C;
          projectConfig.type = firstCommandWord.substr(3);
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

      // prepare c/cpp option list
      {
        List<String> additionalOptions;
        for(const List<String>::Node* i = projectConfig.language == Project::Config::C ? projectConfig.cFlags.getFirst() : projectConfig.cppFlags.getFirst(); i; i = i->getNext())
        {
          projectConfig.compilerFlags.append(i->data);
          const Map<String, Option>::Node* node = knownCppOptions.find(i->data);
          if(node)
          {
            if(!node->data.group)
              continue;
            const Option& option = node->data;
            const OptionGroup& optionGroup = *option.group;
            if(projectConfig.cppOptions.find(optionGroup.name))
            {
              // TODO: warning or error
            }
            else
              projectConfig.cppOptions.append(optionGroup.name, option.value);
            if(option.hasParam(i->data))
            {
              if(projectConfig.cppOptions.find(optionGroup.paramName))
              {
                // TODO: warning or error
              }
              else
                projectConfig.cppOptions.append(optionGroup.paramName, Option::getParamValue(i->data));
            }
            continue;
          }
          node = knownVsOptions.find(i->data);
          if(node)
          {
            projectConfig.vsOptions.append(node->data.group->name, Option::getParamValue(i->data));
            continue;
          }
          additionalOptions.append(i->data);
        }
        if(!additionalOptions.isEmpty())
          projectConfig.cppOptions.append("AdditionalOptions", join(additionalOptions, ' ') + " %(AdditionalOptions)");
      }
      if(projectConfig.vsOptions.find("UseDebugLibraries"))
      { // We do not have to set /Od because of "<UseDebugLibraries>true</UseDebugLibraries>".
        Map<String, String>::Node* node = projectConfig.cppOptions.find("Optimization");
        if(node && node->data == "Disabled")
          projectConfig.cppOptions.remove(node);
      }


      // prepare link option list
      {
        List<String> additionalOptions;
        for(const Map<String, void*>::Node* i = projectConfig.linkFlags.getFirst(); i; i = i->getNext())
        {
          const Map<String, Option>::Node* node = knownLinkOptions.find(i->key);
          if(node)
          {
            if(!node->data.group)
              continue;
            const Option& option = node->data;
            const OptionGroup& optionGroup = *option.group;
            if(projectConfig.linkOptions.find(optionGroup.name))
            {
              // TODO: warning or error
            }
            else
              projectConfig.linkOptions.append(optionGroup.name, option.value);
            continue;
          }
          node = knownVsOptions.find(i->key);
          if(node)
          {
            projectConfig.vsOptions.append(node->data.group->name, Option::getParamValue(i->key));
            continue;
          }
          additionalOptions.append(i->key);
        }
        if(!additionalOptions.isEmpty())
          projectConfig.linkOptions.append("AdditionalOptions", join(additionalOptions, ' ') + " %(AdditionalOptions)");
      }
    }

    // for each file
    for(Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
    {
      const String& fileName = i->key;
      Project::File& file = i->data;

      // for each file configuration
      for(Map<String, Project::File::Config>::Node* i = file.configs.getFirst(); i; i = i->getNext())
      {
        Project::File::Config& fileConfig = i->data;
        Project::Config& projectConfig = project.configs.find(i->key)->data;

        // determine file type
        String firstCommandWord = fileConfig.command.isEmpty() ? String() : Word::first(fileConfig.command.getFirst()->data);
        String type;
        Project::Config::Language language = Project::Config::CPP;
        if(firstCommandWord == "__cppSource")
          type = "ClCompile";
        else if(firstCommandWord == "__cSource")
        {
          language = Project::Config::C;
          type = "ClCompile";
        }
        else if(firstCommandWord == "__rcSource")
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
          {
            file.type = type;
            String basename = File::getBasename(fileName);
            Map<String, int>::Node* node = projectConfig.outputBasenames.find(basename);
            if(node)
            {
              int num = ++(node->data);
              String newExtension;
              newExtension.format(200, "(%d).obj", num);
              file.useProjectCompilerFlags = false;
              fileConfig.cppOptions.append("ObjectFileName", String("$(IntDir)") + File::getWithoutExtension(basename) + newExtension);
            }
            else
              projectConfig.outputBasenames.append(basename, 1);
          }
        }

        // add dependencies of the file to project's dependencies
        for(const List<String>::Node* i = fileConfig.dependencies.getFirst(); i; i = i->getNext())
          if(!project.dependencies.find(i->data))
            project.dependencies.append(i->data);

        // prepare cpp option list
        List<String>* compilerFlags = 0;
        switch(language)
        {
        case Project::Config::C:
          if(fileConfig.hasCFlags)
            compilerFlags = &fileConfig.cFlags;
          break;
        default:
          if(fileConfig.hasCppFlags)
            compilerFlags = &fileConfig.cppFlags;
          break;
        }
        if(!compilerFlags && language != projectConfig.language)
          compilerFlags = language == Project::Config::C ? &projectConfig.cFlags : &projectConfig.cppFlags;
        if(compilerFlags)
        {
          Map<String, void*> fileCompilerFlags;
          for(const List<String>::Node* i = compilerFlags->getFirst(); i; i = i->getNext())
            fileCompilerFlags.append(i->data);

          List<String> additionalOptionsToAdd;
          List<String> additionalOptionsToRemove;
          for(const Map<String, void*>::Node* i = fileCompilerFlags.getFirst(); i; i = i->getNext())
            if(!projectConfig.compilerFlags.find(i->key))
            {
              const Map<String, Option>::Node* node = knownCppOptions.find(i->key);
              if(node)
              {
                if(!node->data.group)
                  continue;
                const Option& option = node->data;
                const OptionGroup& optionGroup = *option.group;
                if(fileConfig.cppOptions.find(optionGroup.name))
                {
                  // TODO: warning or error
                }
                else
                  if(!optionGroup.name.isEmpty())
                    fileConfig.cppOptions.append(optionGroup.name, option.value);
                if(option.hasParam(i->key))
                {
                  if(fileConfig.cppOptions.find(optionGroup.paramName))
                  {
                    // TODO: warning or error
                  }
                  else
                    fileConfig.cppOptions.append(optionGroup.paramName, Option::getParamValue(i->key));
                }
                continue;
              }
              if(knownVsOptions.find(i->key))
                continue; // ignore
              additionalOptionsToAdd.append(i->key);
            }
          for(const Map<String, void*>::Node* i = projectConfig.compilerFlags.getFirst(); i; i = i->getNext())
            if(!fileCompilerFlags.find(i->key))
            {
              const Map<String, Option>::Node* node = knownCppOptions.find(i->key);
              if(node)
              {
                if(!node->data.group)
                  continue;
                const Option& option = node->data;
                const OptionGroup& optionGroup = *option.group;
                if(!optionGroup.unsetValue.isEmpty() && !fileConfig.cppOptions.find(optionGroup.name))
                  fileConfig.cppOptions.append(optionGroup.name, optionGroup.unsetValue);
                continue;
              }
              if(knownVsOptions.find(i->key))
                continue; // ignore
              additionalOptionsToRemove.append(i->key);
            }

          if(!additionalOptionsToRemove.isEmpty())
          { // overwrite project additional options list
            additionalOptionsToAdd.clear();
            for(const Map<String, void*>::Node* i = fileCompilerFlags.getFirst(); i; i = i->getNext())
              if(!knownCppOptions.find(i->key) && !knownVsOptions.find(i->key))
                additionalOptionsToAdd.append(i->key);
            fileConfig.cppOptions.append("AdditionalOptions", join(additionalOptionsToAdd, ' '));
          }
          else if(!additionalOptionsToAdd.isEmpty()) // just add the new options
            fileConfig.cppOptions.append("AdditionalOptions", join(additionalOptionsToAdd, ' ') + " %(AdditionalOptions)");

          if(!fileConfig.cppOptions.isEmpty())
            file.useProjectCompilerFlags = false;
        }
      }

      //
      if(file.configs.getSize() < project.configs.getSize())
        file.useProjectCompilerFlags = false;
    }

    // set special file type for header files
    for(Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
      if(i->data.type.isEmpty())
      {
        String extension = File::getExtension(i->key);
        if(extension == "h" || extension == "hh" || extension == "hxx" || extension == "hpp")
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

bool Vcxproj::resolveDependencies()
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

  // solution dir?
  if(!solutionFile.isEmpty())
  {
    solutionDir = File::getDirname(solutionFile);
    if(solutionDir == ".")
      solutionDir = String();
  }
  else
    solutionFile = solutionName + ".sln";

  // open output file
  if(!solutionDir.isEmpty())
    Directory::create(solutionDir);
  fileOpen(solutionFile);

  // header
  fileWrite("﻿\r\n");
  if(version >= 2013)
    fileWrite("Microsoft Visual Studio Solution File, Format Version 12.00\r\n");
  else
    fileWrite("Microsoft Visual Studio Solution File, Format Version 11.00\r\n");

  if(version == 2015)
    fileWrite("# Visual Studio 14\r\n");
  else if(version == 2013)
    fileWrite("# Visual Studio 2013\r\n");
  else if(version == 2012)
    fileWrite("# Visual Studio 2012\r\n");
  else
    fileWrite("# Visual Studio 2010\r\n");

  // project list
  for(const Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    fileWrite(String("Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"") + i->data.displayName + "\", \"" + relativePath(solutionDir, i->data.projectFile) + "\", \"{" + i->data.guid + "}\"\r\n");
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
  if(!project.projectDir.isEmpty())
    Directory::create(project.projectDir);
  fileOpen(project.projectFile);

  fileWrite("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n");
  if(version == 2015)
    fileWrite("<Project DefaultTargets=\"Build\" ToolsVersion=\"14.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\r\n");
  else if(version == 2013)
    fileWrite("<Project DefaultTargets=\"Build\" ToolsVersion=\"12.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\r\n");
  else
    fileWrite("<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\r\n");

  // write configuration list
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

  // write project name
  fileWrite("  <PropertyGroup Label=\"Globals\">\r\n");
  fileWrite(String("    <ProjectName>") + project.displayName + "</ProjectName>\r\n");
  fileWrite(String("    <ProjectGuid>{") + project.guid + "}</ProjectGuid>\r\n");
  fileWrite(String("    <RootNamespace>") + project.displayName + "</RootNamespace>\r\n");
  fileWrite("  </PropertyGroup>\r\n");

  // write general configuration
  fileWrite("  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\r\n");

  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    const Project::Config& config = i->data;
    fileWrite(String("  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\" Label=\"Configuration\">\r\n");
    fileWrite(String("    <ConfigurationType>") + config.type + "</ConfigurationType>\r\n");

    //if(config.name == "Debug")
    if(config.vsOptions.find("UseDebugLibraries"))
    {
      fileWrite("    <UseDebugLibraries>true</UseDebugLibraries>\r\n"); // This options changes some default compiler/linker settings, but I have no idea how it can be set up in the visual studio project settings.
                                                                        // Let's asume a configuration "uses debug libraries" when _DEBUG is set.
    }
    else
      fileWrite("    <UseDebugLibraries>false</UseDebugLibraries>\r\n");
    if(config.vsOptions.find("PlatformToolset"))
      fileWrite(String("    <PlatformToolset>") + config.vsOptions.lookup("PlatformToolset") + "</PlatformToolset>\r\n");
    else if(version == 2015)
      fileWrite("    <PlatformToolset>v140</PlatformToolset>\r\n");
    else if(version == 2013)
      fileWrite("    <PlatformToolset>v120</PlatformToolset>\r\n");
    else if(version == 2012)
      fileWrite("    <PlatformToolset>v110</PlatformToolset>\r\n");
    if(config.linkFlags.find("/LTCG"))
      fileWrite("    <WholeProgramOptimization>true</WholeProgramOptimization>\r\n");
    if(config.vsOptions.find("CharacterSet"))
      fileWrite(String("    <CharacterSet>") + config.vsOptions.lookup("CharacterSet") + "</CharacterSet>\r\n");

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
      String path = relativePath(project.projectDir, File::getDirname(config.firstOutput));
      fileWrite(String("    <OutDir Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + path + "\\</OutDir>\r\n");
    }
    if(!config.buildDir.isEmpty())
    {
      String path = relativePath(project.projectDir, config.buildDir);
      if(config.firstOutput.isEmpty())
        fileWrite(String("    <OutDir Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + path + "\\</OutDir>\r\n");
      fileWrite(String("    <IntDir Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + path + "\\</IntDir>\r\n");
    }
    if(config.type == "Makefile")
    {
      fileWrite(String("    <NMakeBuildCommandLine Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + joinCommands(config.buildCommand) + "</NMakeBuildCommandLine>\r\n");
      fileWrite(String("    <NMakeReBuildCommandLine Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + joinCommands(config.reBuildCommand) + "</NMakeReBuildCommandLine>\r\n");
      fileWrite(String("    <NMakeCleanCommandLine Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + joinCommands(config.cleanCommand) + "</NMakeCleanCommandLine>\r\n");
      if(!config.outputs.isEmpty())
        fileWrite(String("    <NMakeOutput Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + joinPaths(project.projectDir, config.outputs) + "</NMakeOutput>\r\n");

      fileWrite(String("    <NMakePreprocessorDefinitions Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + join(config.defines) + ";$(NMakePreprocessorDefinitions)</NMakePreprocessorDefinitions>\r\n");
      fileWrite(String("    <NMakeIncludeSearchPath Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + joinPaths(project.projectDir, config.includePaths) + ";$(NMakeIncludeSearchPath)</NMakeIncludeSearchPath>\r\n");

      //fileWrite(String("    <NMakeForcedIncludes Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">$(NMakeForcedIncludes)</NMakeForcedIncludes>\r\n");
      //fileWrite(String("    <NMakeAssemblySearchPath Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">$(NMakeAssemblySearchPath)</NMakeAssemblySearchPath>\r\n");
      //fileWrite(String("    <NMakeForcedUsingAssemblies Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">$(NMakeForcedUsingAssemblies)</NMakeForcedUsingAssemblies>\r\n");
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
      fileWrite(String("    <TargetExt Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + (strchr(basename.getData(), '.') ? String(".") + File::getExtension(basename) : String()) + "</TargetExt>\r\n");
    }
  }

  fileWrite("  </PropertyGroup>\r\n");

  // write configuation settings
  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    const Project::Config& config = i->data;
    fileWrite(String("  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">\r\n");

    if(!config.preBuildCommand.isEmpty())
    {
      fileWrite("    <PreBuildEvent>\r\n");
      fileWrite(String("      <Command>") + joinCommands(config.preBuildCommand) + "</Command>\r\n");
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

    if(config.type != "Makefile")
    {/*
      fileWrite("    <Midl>\r\n");
      fileWrite("      <WarningLevel>0</WarningLevel>\r\n");
      fileWrite("      <DefaultCharType>Unsigned</DefaultCharType>\r\n");
      fileWrite("      <EnableErrorChecks>None</EnableErrorChecks>\r\n");
      fileWrite("    </Midl>\r\n");
      */
      fileWrite("    <ClCompile>\r\n");

      for(const Map<String, String>::Node* i = config.cppOptions.getFirst(); i; i = i->getNext())
        fileWrite(String("      <") + i->key + ">" + i->data + "</" + i->key + ">\r\n");

      if(!config.includePaths.isEmpty())
        fileWrite(String("      <AdditionalIncludeDirectories>") + joinPaths(project.projectDir, config.includePaths) + ";%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\r\n");
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

      for(const Map<String, String>::Node* i = config.linkOptions.getFirst(); i; i = i->getNext())
        fileWrite(String("      <") + i->key + ">" + i->data + "</" + i->key + ">\r\n");

      if(!config.libs.isEmpty())
        fileWrite(String("      <AdditionalDependencies>") + join(config.libs, ';', ".lib") + ";%(AdditionalDependencies)</AdditionalDependencies>\r\n");
      if(!config.libPaths.isEmpty())
        fileWrite(String("      <AdditionalLibraryDirectories>") + joinPaths(project.projectDir, config.libPaths) + ";%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>\r\n");

      fileWrite("    </Link>\r\n");

      /*
      that does not work:
      if(!config.command.isEmpty())
      {
        fileWrite("    <CustomBuild>\r\n");

        if(!config.message.isEmpty())
          fileWrite(String("      <Message>") + xmlEscape(config.message.isEmpty() ? String() : config.message.getFirst()->data) + "</Message>\r\n");
        if(!config.command.isEmpty())
          fileWrite(String("      <Command>") + joinCommands(config.command) + "</Command>\r\n");
        else
        {
          // TODO: warning or error
        }
        if(!config.inputs.isEmpty())
          fileWrite(String("      <AdditionalInputs>") + join(config.inputs) + "</AdditionalInputs>\r\n");

        if(!config.outputs.isEmpty())
          fileWrite(String("      <Outputs>") + join(config.outputs) + "</Outputs>\r\n");
        else
        {
          // TODO: warning or error
        }

        fileWrite("    </CustomBuild>\r\n");
      }
      */
    }
    fileWrite("  </ItemDefinitionGroup>\r\n");
  }

  // write file list
  fileWrite("  <ItemGroup>\r\n");
  for(Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
  {
    Project::File& file = i->data;
    String path = relativePath(project.projectDir, i->key);

    if(file.type == "CustomBuild")
    {
      fileWrite(String("    <CustomBuild Include=\"") + path + "\">\r\n");
      const String& fileName = i->key;
      for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
      {
        Map<String, Project::File::Config>::Node* node = file.configs.find(i->key);
        if(node)
        {
          Project::File::Config& config = node->data;
          if(!config.message.isEmpty())
            fileWrite(String("      <Message Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + xmlEscape(config.message.isEmpty() ? String() : config.message.getFirst()->data) + "</Message>\r\n");
          if(!config.command.isEmpty())
            fileWrite(String("      <Command Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + joinCommands(config.command) + "</Command>\r\n");
          else
          {
            // TODO: warning or error
          }

          List<String> additionalInputs;
          {
            for(const List<String>::Node* i = config.inputs.getFirst(); i; i = i->getNext())
              if(i->data != fileName)
                additionalInputs.append(i->data);
          }

          if(!additionalInputs.isEmpty())
            fileWrite(String("      <AdditionalInputs Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + joinPaths(project.projectDir, additionalInputs) + ";%(AdditionalInputs)</AdditionalInputs>\r\n");

          if(!config.outputs.isEmpty())
            fileWrite(String("      <Outputs Condition=\"'$(Configuration)|$(Platform)'=='") + i->key + "'\">" + joinPaths(project.projectDir, config.outputs) + ";%(Outputs)</Outputs>\r\n");
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
      if(file.useProjectCompilerFlags)
        fileWrite(String("    <") + file.type + " Include=\"" + path + "\" />\r\n");
      else
      {
        fileWrite(String("    <") + file.type + " Include=\"" + path + "\">\r\n");
        // If file configs and project configs are the same nothing has to be excluded and some optimizations can be done (the size should be enough to determine that).
        if(project.configs.getSize() == file.configs.getSize())
        {
          Map<String, String> handledOptions;
          const Map<String, String>& firstCppOptions = file.configs.getFirst()->data.cppOptions;
          for(const Map<String, String>::Node* option = firstCppOptions.getFirst(); option; option = option->getNext())
          {
            const String& optionKey = option->key;
            const String& optionValue = option->data;
            bool equalInAll = true;
            for(const Map<String, Project::File::Config>::Node* otherConfig = file.configs.getFirst(); otherConfig; otherConfig = otherConfig->getNext())
            {
              const Map<String, String>::Node* otherOption = otherConfig->data.cppOptions.find(optionKey);
              if(!otherOption || otherOption->data != optionValue)
              {
                equalInAll = false;
                break;
              }
            }
            if(equalInAll)
            {
              fileWrite(String("      <") + optionKey + ">" + optionValue + "</" + optionKey + ">\r\n");
              handledOptions.append(optionKey, optionValue);
            }
          }
          for(const Map<String, Project::File::Config>::Node* config = file.configs.getFirst(); config; config = config->getNext())
          {
            const String& configKey = config->key;
            const Project::File::Config& fileConfig = config->data;
            if(handledOptions.getSize() != firstCppOptions.getSize() || fileConfig.cppOptions.getSize() != handledOptions.getSize())
              for(const Map<String, String>::Node* option = fileConfig.cppOptions.getFirst(); option; option = option->getNext())
              {
                const String& optionKey = option->key;
                const String& optionValue = option->data;
                if(!handledOptions.find(optionKey))
                  fileWrite(String("      <") + optionKey + " Condition=\"'$(Configuration)|$(Platform)'=='" + configKey + "'\">" + optionValue + "</" + optionKey + ">\r\n");
              }
          }
        }
        else
        {
          for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
          {
            const String& configKey = i->key;
            const Map<String, Project::File::Config>::Node* node = file.configs.find(configKey);
            if(!node)
              fileWrite(String("      <ExcludedFromBuild Condition=\"'$(Configuration)|$(Platform)'=='") + configKey + "'\">true</ExcludedFromBuild>\r\n");
            else
            {
              const Project::File::Config& fileConfig = node->data;
              for(const Map<String, String>::Node* i = fileConfig.cppOptions.getFirst(); i; i = i->getNext())
                fileWrite(String("      <") + i->key + " Condition=\"'$(Configuration)|$(Platform)'=='" + configKey + "'\">" + i->data + "</" + i->key + ">\r\n");
            }
          }
        }
        fileWrite(String("    </") + file.type + ">\r\n");
      }
    }
  }
  fileWrite("  </ItemGroup>\r\n");

  // write dependencies
  if(!project.dependencies.isEmpty())
  {
    fileWrite("  <ItemGroup>\r\n");
    for(const Map<String, void*>::Node* i = project.dependencies.getFirst(); i; i = i->getNext())
    {
      const Map<String, Project>::Node* node = projects.find(i->key);
      if(node)
      {
        const Project& depProject = node->data;
        String path = relativePath(project.projectDir, depProject.projectFile);
        fileWrite(String("    <ProjectReference Include=\"") + path + "\">\r\n");
        fileWrite(String("      <Project>{") + depProject.guid + "}</Project>\r\n");
        fileWrite("      <ReferenceOutputAssembly>false</ReferenceOutputAssembly>\r\n");
        fileWrite("    </ProjectReference>\r\n");
      }
    }
    fileWrite("  </ItemGroup>\r\n");
  }

  // write visualizers
  if(!project.visualizers.isEmpty())
  {
    fileWrite("  <ItemGroup>\r\n");
    for(List<String>::Node* i = project.visualizers.getFirst(); i; i = i->getNext())
    {
      const String path = relativePath(project.projectDir, i->data);
      fileWrite(String("    <Natvis Include=\"") + path + "\" />\r\n");
    }
    fileWrite("  </ItemGroup>\r\n");
  }

  fileWrite("  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\r\n");
  fileWrite("  <ImportGroup Label=\"ExtensionTargets\">\r\n");
  fileWrite("  </ImportGroup>\r\n");
  //fileWrite("</Project>\r\n");
  fileWrite("</Project>");

  fileClose();
  return true;
}

bool Vcxproj::generateVcxprojFilter(Project& project)
{
  fileOpen(project.projectFile + ".filters");
  fileWrite("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n");
  fileWrite("<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\r\n");

  Map<String, void*> roots;
  for(List<String>::Node* i = project.root.getFirst(); i; i = i->getNext())
  {
    String path = File::simplifyPath(i->data);
    path.subst("/", "\\");
    roots.append(path);
  }

  Map<String, String> filters;
  for(Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
  {
    Project::File& file = i->data;

    if(!file.filter.isEmpty()) // a user defined filter
    {
      List<String> filtersToAdd;
      file.filter = File::simplifyPath(file.filter);
      file.filter.subst("/", "\\");
      String filterName = file.filter;
      for(;;)
      {
        filtersToAdd.prepend(filterName);
        filterName = File::getDirname(filterName);
        if(filterName == ".")
          break;
      }
      for(List<String>::Node* i = filtersToAdd.getFirst(); i; i = i->getNext())
      {
        if(!filters.find(i->data))
          filters.append(i->data, createSomethingLikeGUID(i->data));
      }
      continue;
    }

    // create a filter based on the file system hierarchy
    List<String> filtersToAdd;
    String root;
    String filterName = File::getDirname(File::simplifyPath(i->key));
    filterName.subst("/", "\\");
    for(;;)
    {
      if(filterName == "." || File::getBasename(filterName) == "..")
        break;
      const Map<String, void*>::Node* node = roots.find(filterName);
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
          if(strncmp(filterName.getData(), "..\\", 3) == 0)
            filterName = filterName.substr(3);
          else if(strncmp(filterName.getData(), ".\\", 2) == 0)
            filterName = filterName.substr(2);
          else
            break;
        }
      }
      if(!filters.find(filterName))
        filters.append(filterName, createSomethingLikeGUID(filterName));
      if(i == filtersToAdd.getLast())
        file.filter = filterName;
    }
  }
  if(!project.visualizers.isEmpty())
  {
    const String vStr("Visualizers");
    filters.append(vStr, createSomethingLikeGUID("Visualizers"));
  }

  // write filters
  fileWrite("  <ItemGroup>\r\n");
  for(const Map<String, String>::Node* i = filters.getFirst(); i; i = i->getNext())
  {
    fileWrite(String("    <Filter Include=\"") + i->key + "\">\r\n");
    fileWrite(String("      <UniqueIdentifier>{") + i->data + "}</UniqueIdentifier>\r\n");
    fileWrite("    </Filter>\r\n");
  }
  fileWrite("  </ItemGroup>\r\n");

  // write files
  fileWrite("  <ItemGroup>\r\n");
  for(const Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
  {
    const Project::File& file = i->data;
    String path = relativePath(project.projectDir, i->key);
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

  // write visualizers
  if(!project.visualizers.isEmpty())
  {
    fileWrite("  <ItemGroup>\r\n");
    for(List<String>::Node* i = project.visualizers.getFirst(); i; i = i->getNext())
    {
      const String path = relativePath(project.projectDir, i->data);
      fileWrite(String("    <Natvis Include=\"") + path + "\">\r\n");
      fileWrite(String("      <Filter>Visualizers</Filter>\r\n"));
      fileWrite(String("    </Natvis>\r\n"));
    }
    fileWrite("  </ItemGroup>\r\n");
  }

  //fileWrite("</Project>\r\n");
  fileWrite("</Project>");

  fileClose();
  return true;
}

void Vcxproj::fileOpen(const String& name)
{
  if(!file.open(name, File::writeFlag))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
  openedFile = name;
}

void Vcxproj::fileWrite(const String& data)
{
  if(!file.write(data))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
}

void Vcxproj::fileClose()
{
  file.close();
  if(!openedFile.isEmpty())
  {
    puts(openedFile.getData());
    fflush(stdout);
  }
  openedFile.clear();
}

String Vcxproj::createSomethingLikeGUID(const String& name)
{
  // create something like this: 8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942

  MD5 md5;
  md5.update((const unsigned char*)name.getData(), static_cast<unsigned>(name.getLength()));
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

String Vcxproj::relativePath(const String& projectDir, const String& path)
{
  String result;
  if(!projectDir.isEmpty())
  {
    String cwd = Directory::getCurrent() + "/";
    result = File::relativePath(cwd + projectDir, cwd + path);
  }
  else
    result = File::simplifyPath(path);
  result.subst("/", "\\");
  return result;
}

String Vcxproj::join(const List<String>& items, char sep, const String& suffix)
{
  String result;
  const List<String>::Node* i = items.getFirst();
  if(i)
  {
    for(const char* str = i->data.getData(); *str; ++str)
      if(isspace(*(unsigned char*)str))
      {
        result.append('"');
        result.append(xmlEscape(i->data));
        result.append('"');
        goto next;
      }
    result = xmlEscape(i->data);
  next:;
    result.append(suffix);
    for(i = i->getNext(); i; i = i->getNext())
    {
      result.append(sep);
      for(const char* str = i->data.getData(); *str; ++str)
        if(isspace(*(unsigned char*)str))
        {
          result.append('"');
          result.append(xmlEscape(i->data));
          result.append('"');
          goto next2;
        }
      result.append(xmlEscape(i->data));
    next2:;
      result.append(suffix);
    }
  }
  return result;
}

String Vcxproj::joinPaths(const String& projectDir, const List<String>& paths)
{
  if(projectDir.isEmpty())
    return join(paths, ';');
  else
  {
    List<String> relPaths;
    for(const List<String>::Node* i = paths.getFirst(); i; i = i->getNext())
    {
      if(i->data.getData()[0] == '$')
        relPaths.append(i->data);
      else
        relPaths.append(relativePath(projectDir, i->data));
    }
    return join(relPaths, ';');
  }
}

String Vcxproj::joinCommands(const List<String>& commands)
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
          result.append('"');
          result.append(xmlEscape(program));
          result.append('"');
          goto next;
        }
      result.append(xmlEscape(program));
    next:;
      for(i = i->getNext(); i; i = i->getNext())
      {
        result.append(' ');
        for(const char* str = i->data.getData(); *str; ++str)
          if(isspace(*(unsigned char*)str))
          {
            result.append('"');
            result.append(xmlEscape(i->data));
            result.append('"');
            goto next2;
          }
        result.append(xmlEscape(i->data));
      next2:;
      }
    }
  }
  return result;
}

String Vcxproj::xmlEscape(const String& text)
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
    case '<': result.append("&lt;"); break;
    case '>': result.append("&gt;"); break;
    case '&': result.append("&amp;"); break;
    default: result.append(*str); break;
    }
  return result;
}

Map<String, Vcxproj::Option>::Node* Vcxproj::OptionMap::find(const String& flag)
{
  const char* a = strpbrk(flag.getData(), "\"=");
  if(a)
  {
    String optionName = flag.substr(0, a - flag.getData());
    return Map<String, Vcxproj::Option>::find(optionName);
  }
  return Map<String, Vcxproj::Option>::find(flag);
}

bool Vcxproj::Option::hasParam(const String& flag) const
{
  return group && !group->paramName.isEmpty() && strchr(flag.getData(), '"');
}

String Vcxproj::Option::getParamValue(const String& flag)
{
  const char* a = strpbrk(flag.getData(), "\"=");
  if(!a)
    return String();
  if(*a == '=')
  {
    ++a;
    if(*a != '"')
      return String(a, strlen(a));
  }
  ++a;
  size_t len = strlen(a);
  if(a[len - 1] == '"')
    --len;
  return flag.substr(a - flag.getData(), len);
}
