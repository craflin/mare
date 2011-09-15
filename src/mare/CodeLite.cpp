
#include <cstdlib>
#include <cstdio>
#include <cctype>

#include "Engine.h"

#include "Tools/Assert.h"
#include "Tools/Error.h"
#include "Tools/Word.h"

#include "CodeLite.h"

bool CodeLite::generate(const Map<String, String>& userArgs)
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

  // generate project files
  if(!generateProjects())
    return false;

  return true;
}

bool CodeLite::readFile()
{
  // get some global keys
  engine.enterRootKey();
  workspaceName = engine.getFirstKey("name");
  List<String> allPlatforms, allConfigurations, allTargets;
  engine.getKeys("platforms", allPlatforms);
  engine.getKeys("configurations", allConfigurations);
  engine.getKeys("targets", allTargets);
  engine.leaveUnnamedKey();

  // do something for each target in each configuration
  for(const List<String>::Node* i = allPlatforms.getFirst(); i; i = 0) // just use the first platform since CodeLite does not really support multiple target platforms
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

        engine.getKeys("buildCommand", projectConfig.buildCommand, false);
        engine.getKeys("reBuildCommand", projectConfig.reBuildCommand, false);
        engine.getKeys("cleanCommand", projectConfig.cleanCommand, false);
        projectConfig.buildDir = engine.getFirstKey("buildDir", true);

        engine.getKeys("command", projectConfig.command, false);
        projectConfig.firstOutput = engine.getFirstKey("outputs", false);

        if(!projectConfig.command.isEmpty())
        {
          String firstCommand = projectConfig.command.getFirst()->data;
          if(firstCommand == "__Custom" || firstCommand == "__Application" || firstCommand == "__StaticLibrary" || firstCommand == "__DynamicLibrary")
          {
            if(firstCommand == "__Custom")
            {
              projectConfig.customBuild = true;
              firstCommand = (projectConfig.command.getSize() > 1) ? projectConfig.command.getFirst()->getNext()->data : String();
            }

            if(firstCommand == "__Application")
              projectConfig.type = "Executable";
            else if(firstCommand == "__StaticLibrary")
              projectConfig.type = "Static Library";
            else if(firstCommand == "__DynamicLibrary")
              projectConfig.type = "Dynamic Library";
            projectConfig.command.clear();
          }
        }
        if(!projectConfig.buildCommand.isEmpty())
          projectConfig.customBuild = true;

        /*
        engine.getKeys("cppFlags", projectConfig.cppFlags, true);
        List<String> linkFlags;
        engine.getKeys("linkFlags", linkFlags, true);
        for(const List<String>::Node* i = linkFlags.getFirst(); i; i = i->getNext())
          projectConfig.linkFlags.append(i->data, 0);
        projectConfig.firstOutput = engine.getFirstKey("outputs", false);
        engine.getKeys("defines", projectConfig.defines, true);
        engine.getKeys("includePaths", projectConfig.includePaths, true);
        engine.getKeys("libPaths", projectConfig.libPaths, true);
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
            engine.leaveUnnamedKey();
          }

          engine.leaveKey();
        }

        engine.leaveKey();
        engine.leaveKey();
        engine.leaveUnnamedKey();
        engine.leaveUnnamedKey();
      }
    }
  }

  return true;
}

bool CodeLite::generateWorkspace()
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
  fileOpen(workspaceName + ".workspace");

  // write
  fileWrite("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
  fileWrite(String("<CodeLite_Workspace Name=\"") + workspaceName+ "\" Database=\"./" + workspaceName + ".tags\">\n");
  for(const Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    const Project& project = i->data;
    fileWrite(String("  <Project Name=\"") + project.name + "\" Path=\"" + project.name + ".project\"" + (i == projects.getFirst() ? String(" Active=\"Yes\"") : String()) + "/>\n");
  }
  fileWrite("  <BuildMatrix>\n");
  for(const Map<String, void*>::Node* i = configs.getFirst(); i; i = i->getNext())
  {
    const String& configName = i->key;

    fileWrite(String("    <WorkspaceConfiguration Name=\"") + configName + "\" Selected=\"yes\">\n");
    for(const Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
    {
      const Project& project = i->data;
      fileWrite(String("      <Project Name=\"") + project.name + "\" ConfigName=\"" + configName + "\"/>\n");
    }
    fileWrite("    </WorkspaceConfiguration>\n");
  }
  fileWrite("  </BuildMatrix>\n");
  fileWrite("</CodeLite_Workspace>\n");

  //
  fileClose();
  return true;
}

bool CodeLite::generateProjects()
{
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
    if(!generateProject(i->data))
      return false;
  return true;
}

bool CodeLite::generateProject(Project& project)
{
  fileOpen(project.name + ".project");

  fileWrite("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
  fileWrite(String("<CodeLite_Project Name=\"") + project.name + "\" InternalType=\"Console\">\n");
  fileWrite("  <Description/>\n");

  class FileTree
  {
  public:
    Map<String, void*> folders;
    List<String> files;

    FileTree()
    {
      for(Map<String, void*>::Node* i = folders.getFirst(); i; i = i->getNext())
        delete (FileTree*)i->data;
    }

    void addFile(Project& project, const String& file, const String& folder)
    {
      List<String> foldersToEnter;
      String dirName = folder.isEmpty() ? File::getDirname(file) : folder;
      for(;;)
      {
        if(dirName == ".")
          break;
        String dirBaseName = File::getBasename(dirName);
        if(folder.isEmpty() && (dirBaseName == ".." || project.roots.find(dirName)))
          break;
        foldersToEnter.prepend(dirBaseName);
        dirName = File::getDirname(dirName);
      }
      FileTree* f = this;
      for(const List<String>::Node* i = foldersToEnter.getFirst(); i; i = i->getNext())
      {
        Map<String, void*>::Node* node = f->folders.find(i->data);
        f = (FileTree*)(node ? node->data : f->folders.append(i->data, new FileTree()));
      }
      f->files.append(file);
    }

    void write(CodeLite& codeLite, const String& space) const
    {
      for(const Map<String, void*>::Node* i = folders.getFirst(); i; i = i->getNext())
      {
        codeLite.fileWrite(space + "  <VirtualDirectory Name=\"" + i->key + "\">\n");
        ((FileTree*)i->data)->write(codeLite, space + "  ");
        codeLite.fileWrite(space + "  </VirtualDirectory>\n");
      }
      for(const List<String>::Node* i = files.getFirst(); i; i = i->getNext())
        codeLite.fileWrite(space + "  <File Name=\"" + i->data + "\"/>\n");
    }
  } fileTree;

  for(Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
    fileTree.addFile(project, i->key, i->data.folder);

  fileTree.write(*this, String());

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

  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    const Project::Config& config = i->data;
    fileWrite(String("    <Configuration Name=\"") + config.name + "\" CompilerType=\"gnu g++\" DebuggerType=\"GNU gdb debugger\" Type=\"" + config.type + "\" BuildCmpWithGlobalSettings=\"append\" BuildLnkWithGlobalSettings=\"append\" BuildResWithGlobalSettings=\"append\">\n");
    if(config.customBuild)
      fileWrite("      <Compiler Options=\"\" C_Options=\"\" Required=\"no\" PreCompiledHeader=\"\"/>\n");
    else
    { // TODO
      fileWrite("      <Compiler Options=\"-g\" C_Options=\"-g\" Required=\"yes\" PreCompiledHeader=\"\">\n");
      fileWrite("        <IncludePath Value=\".\"/>\n");
      fileWrite("      </Compiler>\n");
    }
    if(config.customBuild)
      fileWrite("      <Linker Options=\"\" Required=\"no\"/>\n");
    else
    {
      // TODO
      fileWrite("      <Linker Options=\"\" Required=\"yes\"/>\n");
    }

    // TODO
    fileWrite("      <ResourceCompiler Options=\"\" Required=\"no\"/>\n");

    fileWrite(String("      <General OutputFile=\"") + config.firstOutput + "\" IntermediateDirectory=\"" + config.buildDir + "\" Command=\"./" + config.firstOutput + "\" CommandArguments=\"\" UseSeparateDebugArgs=\"no\" DebugArguments=\"\" WorkingDirectory=\".\" PauseExecWhenProcTerminates=\"yes\"/>\n");

    fileWrite("      <Environment EnvVarSetName=\"&lt;Use Defaults&gt;\" DbgSetName=\"&lt;Use Defaults&gt;\"/>\n");
    fileWrite("      <Debugger IsRemote=\"no\" RemoteHostName=\"\" RemoteHostPort=\"\" DebuggerPath=\"\">\n");
    fileWrite("        <PostConnectCommands/>\n");
    fileWrite("        <StartupCommands/>\n");
    fileWrite("      </Debugger>\n");
    fileWrite("      <PreBuild/>\n");
    fileWrite("      <PostBuild/>\n");
    fileWrite(String("      <CustomBuild Enabled=\"")+ (config.customBuild ? String("yes") : String("no")) + "\">\n");
    fileWrite(String("        <RebuildCommand>") + joinCommands(config.reBuildCommand) + "</RebuildCommand>\n");
    fileWrite(String("        <CleanCommand>") + joinCommands(config.cleanCommand) + "</CleanCommand>\n");
    fileWrite(String("        <BuildCommand>") + joinCommands(config.buildCommand) + "</BuildCommand>\n");
    fileWrite("        <PreprocessFileCommand/>\n");
    fileWrite("        <SingleFileCommand/>\n");
    fileWrite("        <MakefileGenerationCommand/>\n");
    fileWrite("        <ThirdPartyToolName>None</ThirdPartyToolName>\n");
    fileWrite("        <WorkingDirectory/>\n");
    fileWrite("      </CustomBuild>\n");
    fileWrite("      <AdditionalRules>\n");
    fileWrite("        <CustomPostBuild/>\n");
    fileWrite("        <CustomPreBuild/>\n");
    fileWrite("      </AdditionalRules>\n");
    fileWrite("    </Configuration>\n");
  }

  fileWrite("  </Settings>\n");

  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    const Project::Config& config = i->data;
    fileWrite(String("  <Dependencies Name=\"") + config.name + "\">\n");
    for(const Map<String, void*>::Node* i = project.dependencies.getFirst(); i; i = i->getNext())
    {
      fileWrite(String("    <Project Name=\"") + i->key + "\"/>\n");
    }
    fileWrite("  </Dependencies>\n");
  }

  fileWrite("</CodeLite_Project>\n");

  fileClose();
  return true;
}

void CodeLite::fileOpen(const String& name)
{
  if(!file.open(name, File::writeFlag))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
  openedFile = name;
}

void CodeLite::fileWrite(const String& data)
{
  if(!file.write(data))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
}

void CodeLite::fileClose()
{
  file.close();
  if(!openedFile.isEmpty())
  {
    puts(openedFile.getData());
    fflush(stdout);
  }
  openedFile.clear();
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
  return Word::join(commands);
  // TODO: something for more than a single command?
}

String CodeLite::xmlEscape(const String& text)
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
