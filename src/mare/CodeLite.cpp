
#include <cstdlib>
#include <cstdio>
#include <cctype>

#include "Engine.h"

#include "Tools/Assert.h"
#include "Tools/Error.h"
#include "Tools/Word.h"

#include "CodeLite.h"

void CodeLite::addDefaultKeys(Engine& engine)
{
  engine.addDefaultKey("tool", "CodeLite");
  engine.addDefaultKey("CodeLite", "CodeLite");
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
  engine.addDefaultKey("cFlags", "-Wall $(if $(Debug),-g,-Os -fomit-frame-pointer)");
  engine.addDefaultKey("cppFlags", "-Wall $(if $(Debug),-g,-Os -fomit-frame-pointer)");
  engine.addDefaultKey("linkFlags", "$(if $(Debug),,-s)");

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
    cApplication.append("output", "$(outputDir)/$(target)$(if $(Win32),.exe)");
    engine.addDefaultKey("cApplication", cApplication);
  }
  {
    Map<String, String> cppApplication;
    cppApplication.append("command", "__cppApplication");
    cppApplication.append("output", "$(outputDir)/$(target)$(if $(Win32),.exe)");
    engine.addDefaultKey("cppApplication", cppApplication);
  }
  {
    Map<String, String> cDynamicLibrary;
    cDynamicLibrary.append("command", "__cDynamicLibrary");
    cDynamicLibrary.append("output", "$(outputDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.dll,.so)");
    engine.addDefaultKey("cDynamicLibrary", cDynamicLibrary);
  }
  {
    Map<String, String> cppDynamicLibrary;
    cppDynamicLibrary.append("command", "__cppDynamicLibrary");
    cppDynamicLibrary.append("output", "$(outputDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.dll,.so)");
    engine.addDefaultKey("cppDynamicLibrary", cppDynamicLibrary);
  }
  {
    Map<String, String> cStaticLibrary;
    cStaticLibrary.append("command", "__cStaticLibrary");
    cStaticLibrary.append("output", "$(outputDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.lib,.a)");
    engine.addDefaultKey("cStaticLibrary", cStaticLibrary);
  }
  {
    Map<String, String> cppStaticLibrary;
    cppStaticLibrary.append("command", "__cppStaticLibrary");
    cppStaticLibrary.append("output", "$(outputDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.lib,.a)");
    engine.addDefaultKey("cppStaticLibrary", cppStaticLibrary);
  }
}

bool CodeLite::processData(const Data& data)
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
        configurationName += String(" ") + platformName;
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
        configurationName += String(" ") + platformName;
      const Configuration& configuration = i->data;
      for(const Map<String, Target>::Node* i = configuration.targets.getFirst(); i; i = i->getNext())
      {
        const String& targetName = i->key;
        const Target& target = i->data;
        Map<String, Project>::Node* node = projects.find(targetName);
        Project& project = node ? node->data : projects.append(targetName);
        ProjectConfiguration& projectConfig = project.configurations.append(configurationName);
        projectConfig.target = &target;

        for(const Map<String, File>::Node* i = target.files.getFirst(); i; i = i->getNext())
        {
          const String& fileName = i->key;
          Map<String, ProjectFile>::Node* node = project.files.find(fileName);
          ProjectFile& file = node ? node->data : project.files.append(fileName);
          ProjectFile::Configuration& fileConfig = file.configurations.append(configurationName);
          fileConfig.file = &i->data;
        }
      }
    }
  }

  // remove projects not creating any output files
  for(Map<String, Project>::Node* i = projects.getFirst(), * nexti; i; i = nexti)
  {
    nexti = i->getNext();
    if(!i->data.files.isEmpty())
      continue;
    for(const Map<String, ProjectConfiguration>::Node* j = i->data.configurations.getFirst(); j; j = j->getNext())
    {
      const Target& target = *j->data.target;
      if(!target.command.isEmpty() || !target.output.isEmpty())
        goto next;
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

  // generate file tree
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    Project& project = i->data;

    // get root directories
    for(Map<String, ProjectConfiguration>::Node* i = project.configurations.getFirst(); i; i = i->getNext())
    {
      const ProjectConfiguration& configuration = i->data;
      const Target& target = *configuration.target;
      for(const List<String>::Node* i = target.root.getFirst(); i; i = i->getNext())
        project.roots.append(i->data);
    }

    // get folder of each file
    for(Map<String, ProjectFile>::Node* i = project.files.getFirst(); i; i = i->getNext())
    {
      ProjectFile& file = i->data;
      for(const Map<String, ProjectFile::Configuration>::Node* i = file.configurations.getFirst(); i; i = i->getNext())
        if(!i->data.file->folder.isEmpty())
        {
          file.folder = i->data.file->folder;
          break;
        }
    }

    // create folders and add files to the folders
    for(const Map<String, ProjectFile>::Node* i = project.files.getFirst(); i; i = i->getNext())
    {
      const String& fileName = i->key;
      const ProjectFile& file = i->data;
      const String& folder = i->data.folder;
      List<String> foldersToEnter;
      String dirName = folder.isEmpty() ? ::File::getDirname(fileName) : folder;
      for(;;)
      {
        if(dirName == ".")
          break;
        String dirBaseName = ::File::getBasename(dirName);
        if(folder.isEmpty() && (dirBaseName == ".." || project.roots.find(dirName)))
          break;
        foldersToEnter.prepend(dirBaseName);
        dirName = ::File::getDirname(dirName);
      }
      FileTreeNode* f = &project.fileTree;
      for(const List<String>::Node* i = foldersToEnter.getFirst(); i; i = i->getNext())
      {
        Map<String, FileTreeNode*>::Node* node = f->folders.find(i->data);
        f = node ? node->data : f->folders.append(i->data, new FileTreeNode());
      }
      f->files.append(fileName, &file);
    }
  }

/*
  // determine project types
  for(Map<String, Project>::Node* i = projects.getFirst(), * nexti; i; i = nexti)
  {
    Project& project = i->data;
    for(Map<String, Project::Configuration>::Node* i = project.configurations.getFirst(); i; i = i->getNext())
    {
      Project::Configuration& configuration = i->data;

    }
  }
  */

  return true;
}

bool CodeLite::writeFiles()
{
  if(!writeWorkspace())
    return false;
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
    if(!writeProject(i->key, i->data))
      return false;
  return true;
}

bool CodeLite::writeWorkspace()
{
  // open output file
  fileOpen(workspaceName + ".workspace");

  // write
  fileWrite("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
  fileWrite(String("<CodeLite_Workspace Name=\"") + workspaceName+ "\" Database=\"./" + workspaceName + ".tags\">\n");
  for(const Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    const String& projectName = i->key;
    fileWrite(String("  <Project Name=\"") + projectName + "\" Path=\"" + projectName + ".project\"" + (i == projects.getFirst() ? String(" Active=\"Yes\"") : String()) + "/>\n");
  }
  fileWrite("  <BuildMatrix>\n");
  for(const List<String>::Node* i = configurations.getFirst(); i; i = i->getNext())
  {
    const String& configurationName = i->data;

    fileWrite(String("    <WorkspaceConfiguration Name=\"") + configurationName + "\" Selected=\"yes\">\n");
    for(const Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
    {
      const String& projectName = i->key;
      fileWrite(String("      <Project Name=\"") + projectName + "\" ConfigName=\"" + configurationName + "\"/>\n");
    }
    fileWrite("    </WorkspaceConfiguration>\n");
  }
  fileWrite("  </BuildMatrix>\n");
  fileWrite("</CodeLite_Workspace>\n");

  //
  fileClose();
  return true;
}

bool CodeLite::writeProject(const String& projectName, const Project& project)
{
  fileOpen(projectName + ".project");

  fileWrite("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
  fileWrite(String("<CodeLite_Project Name=\"") + projectName + "\" InternalType=\"Console\">\n");
  fileWrite("  <Description/>\n");

  class FileTreeWriter
  {
  public:
    static void write(const List<String>& configurations, const FileTreeNode& node, CodeLite& codeLite, const String& space)
    {
      for(const Map<String, FileTreeNode*>::Node* i = node.folders.getFirst(); i; i = i->getNext())
      {
        codeLite.fileWrite(space + "  <VirtualDirectory Name=\"" + i->key + "\">\n");
        write(configurations, *i->data, codeLite, space + "  ");
        codeLite.fileWrite(space + "  </VirtualDirectory>\n");
      }

      List<String> excludeConfigurations;
      for(const Map<String, const ProjectFile*>::Node* i = node.files.getFirst(); i; i = i->getNext())
      {
        const String& fileName = i->key;
        const ProjectFile& file = *i->data;
        String fileExtension = ::File::getExtension(fileName);
        fileExtension.lowercase();
        excludeConfigurations.clear();
        for(const List<String>::Node* i = configurations.getFirst(); i; i = i->getNext())
        {
          const Map<String, ProjectFile::Configuration>::Node* node = file.configurations.find(i->data);
          if(!node)
          {
            excludeConfigurations.append(i->data);
            continue;
          }
          const ProjectFile::Configuration& fileConfig = node->data;
          String command;
          if(!fileConfig.file->command.isEmpty())
            command = fileConfig.file->command.getFirst()->data;
          if(command == "__cSource" || command == "__cppSource" || command == "__rcSource")
            continue;
          if(fileExtension == "c" || fileExtension == "cc" || fileExtension == "cpp" || fileExtension == "cxx" || fileExtension == "rc")
          {
            excludeConfigurations.append(i->data);
            continue;
          }
        }

        if(excludeConfigurations.isEmpty())
          codeLite.fileWrite(space + "  <File Name=\"" + fileName + "\"/>\n");
        else
          codeLite.fileWrite(space + "  <File Name=\"" + fileName + "\" ExcludeProjConfig=\"" + join(excludeConfigurations) + "\"/>\n");
      }
    }
  };

  FileTreeWriter::write(configurations, project.fileTree, *this, String());

  fileWrite("  <Settings Type=\"Executable\">\n");

  fileWrite("    <GlobalSettings>\n");
  fileWrite("      <Compiler Options=\"\" C_Options=\"\">\n");
  fileWrite("        <IncludePath Value=\".\"/>\n");
  fileWrite("      </Compiler>\n");
  fileWrite("      <Linker Options=\"\">\n");
  fileWrite("        <LibraryPath Value=\".\"/>\n");
  fileWrite("      </Linker>\n");
  fileWrite("      <ResourceCompiler Options=\"\"/>\n");
  fileWrite("    </GlobalSettings>\n");

  for(const Map<String, ProjectConfiguration>::Node* i = project.configurations.getFirst(); i; i = i->getNext())
  {
    const String& configurationName = i->key;
    const ProjectConfiguration& configuration = i->data;
    const Target& target = *configuration.target;

    String firstCommand;
    if(!target.command.isEmpty())
      firstCommand = target.command.getFirst()->data;
    String type = "Executable";
    if(firstCommand == "__cApplication" || firstCommand == "__cppApplication")
      type = "Executable";
    else if(firstCommand == "__cStaticLibrary" || firstCommand == "__cppStaticLibrary")
      type = "Static Library";
    else if(firstCommand == "__cDynamicLibrary" || firstCommand == "__cppDynamicLibrary")
      type = "Dynamic Library";

    bool customBuild = !configuration.target->buildCommand.isEmpty();

    fileWrite(String("    <Configuration Name=\"") + configurationName + "\" CompilerType=\"gnu g++\" DebuggerType=\"GNU gdb debugger\" Type=\"" + type + "\" BuildCmpWithGlobalSettings=\"append\" BuildLnkWithGlobalSettings=\"append\" BuildResWithGlobalSettings=\"append\">\n");
    if(customBuild)
      fileWrite("      <Compiler Options=\"\" C_Options=\"\" Required=\"no\" PreCompiledHeader=\"\"/>\n");
    else
    {
      if(firstCommand == "__cDynamicLibrary" || firstCommand == "__cppDynamicLibrary")
        fileWrite(String("      <Compiler Options=\"-fpic ") + join(target.cppFlags, ' ') + "\" C_Options=\"-fpic " + join(target.cFlags, ' ') +  "\" Required=\"yes\" PreCompiledHeader=\"\">\n");
      else
        fileWrite(String("      <Compiler Options=\"") + join(target.cppFlags, ' ') + "\" C_Options=\"" + join(target.cFlags, ' ') +  "\" Required=\"yes\" PreCompiledHeader=\"\">\n");
      for(const List<String>::Node* i = target.includePaths.getFirst(); i; i = i->getNext())
        fileWrite(String("        <IncludePath Value=\"") + xmlEscape(i->data) + "\"/>\n");
      for(const List<String>::Node* i = target.defines.getFirst(); i; i = i->getNext())
        fileWrite(String("        <Preprocessor Value=\"") + xmlEscape(i->data) + "\"/>\n");
      fileWrite("      </Compiler>\n");
    }
    if(customBuild)
      fileWrite("      <Linker Options=\"\" Required=\"no\"/>\n");
    else
    {
      fileWrite(String("      <Linker Options=\"") + join(target.linkFlags, ' ') + "\" Required=\"yes\">\n");
      for(const List<String>::Node* i = target.libPaths.getFirst(); i; i = i->getNext())
        fileWrite(String("        <LibraryPath Value=\"") + xmlEscape(i->data) + "\"/>\n");
      for(const List<String>::Node* i = target.libs.getFirst(); i; i = i->getNext())
        fileWrite(String("        <Library Value=\"") + xmlEscape(i->data) + "\"/>\n");
      fileWrite("      </Linker>\n");
    }

    if(customBuild)
      fileWrite("      <ResourceCompiler Options=\"\" Required=\"no\"/>\n");
    else
      fileWrite("      <ResourceCompiler Options=\"\" Required=\"yes\"/>\n");

    String firstOutput;
    if(!target.output.isEmpty())
      firstOutput = target.output.getFirst()->data;
    fileWrite(String("      <General OutputFile=\"") + firstOutput + "\" IntermediateDirectory=\"" + target.buildDir + "\" Command=\"./" + firstOutput + "\" CommandArguments=\"\" UseSeparateDebugArgs=\"no\" DebugArguments=\"\" WorkingDirectory=\".\" PauseExecWhenProcTerminates=\"yes\"/>\n");

    fileWrite("      <Environment EnvVarSetName=\"&lt;Use Defaults&gt;\" DbgSetName=\"&lt;Use Defaults&gt;\"/>\n");
    fileWrite("      <Debugger IsRemote=\"no\" RemoteHostName=\"\" RemoteHostPort=\"\" DebuggerPath=\"\">\n");
    fileWrite("        <PostConnectCommands/>\n");
    fileWrite("        <StartupCommands/>\n");
    fileWrite("      </Debugger>\n");
    fileWrite("      <PreBuild/>\n");
    fileWrite("      <PostBuild/>\n");
    fileWrite(String("      <CustomBuild Enabled=\"")+ (customBuild ? String("yes") : String("no")) + "\">\n");
    fileWrite(String("        <RebuildCommand>") + joinCommands(target.reBuildCommand) + "</RebuildCommand>\n");
    fileWrite(String("        <CleanCommand>") + joinCommands(target.cleanCommand) + "</CleanCommand>\n");
    fileWrite(String("        <BuildCommand>") + joinCommands(target.buildCommand) + "</BuildCommand>\n");
    fileWrite("        <PreprocessFileCommand/>\n");
    fileWrite("        <SingleFileCommand/>\n");
    fileWrite("        <MakefileGenerationCommand/>\n");
    fileWrite("        <ThirdPartyToolName>None</ThirdPartyToolName>\n");
    fileWrite("        <WorkingDirectory/>\n");
    fileWrite("      </CustomBuild>\n");
    fileWrite("      <AdditionalRules>\n");
    fileWrite("        <CustomPostBuild/>\n");
    //fileWrite("        <CustomPreBuild/>\n");

    String customPreBuildRules, customPreBuildDeps;
    for(const Map<String, ProjectFile>::Node* i = project.files.getFirst(); i; i = i->getNext())
    {
      const ProjectFile& projectFile = i->data;
      const Map<String, ProjectFile::Configuration>::Node* node = projectFile.configurations.find(configurationName);
      if(!node)
        continue;
      const File& file = *node->data.file;
      String firstCommand;
      if(!file.command.isEmpty())
        firstCommand = file.command.getFirst()->data;
      if(firstCommand.isEmpty() || firstCommand == "__cSource" || firstCommand == "__cppSource" || firstCommand == "__rcSource")
        continue;
      if(file.output.isEmpty())
        continue; // todo: warning?
      const String& firstOutput = file.output.getFirst()->data;
      customPreBuildDeps += firstOutput + " ";
      customPreBuildRules  += firstOutput + ": " + join(file.input, ' ') + "\n";
      customPreBuildRules  += String("\t") + joinCommands(file.command) + "\n";
    }

    fileWrite(String("        <CustomPreBuild>") + customPreBuildDeps + "\n" + customPreBuildRules + "</CustomPreBuild>\n");
    fileWrite("      </AdditionalRules>\n");
    fileWrite("    </Configuration>\n");
  }

  fileWrite("  </Settings>\n");

  for(const Map<String, ProjectConfiguration>::Node* i = project.configurations.getFirst(); i; i = i->getNext())
  {
    const String& configurationName = i->key;
    const ProjectConfiguration& configuration = i->data;
    const Target& target = *configuration.target;
    fileWrite(String("  <Dependencies Name=\"") + configurationName + "\">\n");
    for(const List<String>::Node* i = target.dependencies.getFirst(); i; i = i->getNext())
    {
      // todo: check if dependency exists
      fileWrite(String("    <Project Name=\"") + i->data + "\"/>\n");
    }
    fileWrite("  </Dependencies>\n");
  }

  fileWrite("</CodeLite_Project>\n");

  fileClose();
  return true;
}


String CodeLite::join(const List<String>& items, char sep, const String& suffix)
{
  String result;
  const List<String>::Node* i = items.getFirst();
  if(i)
  {
    result = xmlEscape(i->data);
    result.append(suffix);
    for(i = i->getNext(); i; i = i->getNext())
    {
      result.append(sep);
      result.append(xmlEscape(i->data));
      result.append(suffix);
    }
  }
  return result;
}

String CodeLite::joinCommands(const List<String>& commands)
{
  String result;
  for(const List<String>::Node* i = commands.getFirst(); i; i = i->getNext())
  {
    if(i->data.isEmpty())
      continue;
    if(!result.isEmpty())
      result.append(" &amp;&amp; ");
    result.append(xmlEscape(i->data));
  }
  return result;
}

String CodeLite::xmlEscape(const String& text)
{
  const char* str = text.getData();
  for(; *str; ++str)
    if(*str == '"' || *str == '<' || *str == '>' || *str == '&')
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
