
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cstring>

#include "Engine.h"

#include "Tools/Assert.h"
#include "Tools/Error.h"
#include "Tools/Word.h"

#include "CodeBlocks.h"

bool CodeBlocks::generate(const Map<String, String>& userArgs)
{
  engine.addDefaultKey("tool", "CodeBlocks");
  engine.addDefaultKey("CodeBlocks", "CodeBlocks");
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

bool CodeBlocks::readFile()
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
  for(const List<String>::Node* i = allPlatforms.getFirst(); i; i = 0) // just use the first platform since CodeBlocks does not really support multiple target platforms
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

        engine.getText("command", projectConfig.command, false);
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

bool CodeBlocks::generateWorkspace()
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
  fileWrite("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n");
  fileWrite("<CodeBlocks_workspace_file>\n");
  fileWrite(String("\t<Workspace title=\"") + workspaceName + "\">\n");
  for(const Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    const Project& project = i->data;
    fileWrite(String("\t\t<Project filename=\"") + project.name + ".cbp\"" + (i == projects.getFirst() ? String(" active=\"1\"") : String()) + ">\n");
    for(const Map<String, void*>::Node* i = project.dependencies.getFirst(); i; i = i->getNext())
      fileWrite(String("\t\t\t<Depends filename=\"") + i->key + ".cbp\" />\n");
    fileWrite("\t\t</Project>\n");
  }
  fileWrite("\t</Workspace>\n");
  fileWrite("</CodeBlocks_workspace_file>\n");

  //
  fileClose();
  return true;
}

bool CodeBlocks::generateProjects()
{
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
    if(!generateProject(i->data))
      return false;
  return true;
}

bool CodeBlocks::generateProject(Project& project)
{
  fileOpen(project.name + ".cbp");

  fileWrite("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n");
  fileWrite("<CodeBlocks_project_file>\n");
  fileWrite("\t<FileVersion major=\"1\" minor=\"6\" />\n");

  fileWrite("\t<Project>\n");
  fileWrite(String("\t\t<Option title=\"") + project.name + "\" />\n");
  fileWrite(String("\t\t<Option makefile=\"") + project.name + ".make\" />\n");
  fileWrite("\t\t<Option makefile_is_custom=\"1\" />\n");
  fileWrite("\t\t<Option execution_dir=\".\" />\n");
  fileWrite("\t\t<Option pch_mode=\"2\" />\n");
  fileWrite("\t\t<Option compiler=\"gcc\" />\n");

  Map<String, void*> virtualFolders;
  for(Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
  {
    Project::File& file = i->data;

    if(!file.folder.isEmpty()) // a user defined filter
    {
      if(!virtualFolders.find(file.folder))
        virtualFolders.append(file.folder);
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
        filtersToAdd.prepend(filterName);
        root = File::getDirname(node->key);
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
      if(!virtualFolders.find(filterName))
        virtualFolders.append(filterName);
      if(i == filtersToAdd.getLast())
        file.folder = filterName;
    }
  }


  String virtualFolderStr;
  for(const Map<String, void*>::Node* i = virtualFolders.getFirst(); i; i = i->getNext())
  {
    virtualFolderStr.append(i->key);
    virtualFolderStr.append("/;");
  }
  fileWrite(String("\t\t<Option virtualFolders=\"") + virtualFolderStr + "\" />\n");


  fileWrite("\t\t<Build>\n");

  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    const Project::Config& config = i->data;
    fileWrite(String("\t\t\t<Target title=\"") + config.name + "\">\n");
    fileWrite(String("\t\t\t\t<Option output=\"") + config.firstOutput + "\" prefix_auto=\"1\" extension_auto=\"1\" />\n");
    fileWrite(String("\t\t\t\t<Option object_output=\"") + config.buildDir + "\" />\n");
    fileWrite("\t\t\t\t<Option type=\"0\" />\n");
    fileWrite("\t\t\t\t<Option compiler=\"gcc\" />\n");
    if(config.customBuild)
    {
      fileWrite("\t\t\t\t<MakeCommands>\n");
      String buildCommand = joinCommands(config.buildCommand);
      fileWrite(String("\t\t\t\t\t<Build command=\"") + buildCommand + "\" />\n");
      fileWrite("\t\t\t\t\t<CompileFile command=\"\" />\n");
      fileWrite(String("\t\t\t\t\t<Clean command=\"") + joinCommands(config.cleanCommand) + "\" />\n");
      fileWrite("\t\t\t\t\t<DistClean command=\"\" />\n");
      fileWrite("\t\t\t\t\t<AskRebuildNeeded command=\"exit 1\" />\n");
      fileWrite(String("\t\t\t\t\t<SilentBuild command=\"") + buildCommand + "\" />\n");
      fileWrite("\t\t\t\t</MakeCommands>\n");
    }
    fileWrite("\t\t\t</Target>\n");
  }
  fileWrite("\t\t</Build>\n");

  for(const Map<String, Project::File>::Node* i = project.files.getFirst(); i; i = i->getNext())
  {
    const Project::File& file = i->data;
    if(file.folder.isEmpty())
      fileWrite(String("\t\t<Unit filename=\"") + file.name + "\" />\n");
    else
    {
      fileWrite(String("\t\t<Unit filename=\"") + file.name + "\">\n");
      fileWrite(String("\t\t\t<Option virtualFolder=\"") + file.folder + "/\" />\n");
      fileWrite("\t\t</Unit>\n");
    }
  }

  fileWrite("\t\t<Extensions>\n");
  fileWrite("\t\t\t<code_completion />\n");
  fileWrite("\t\t\t<debugger />\n");
  fileWrite("\t\t</Extensions>\n");
  fileWrite("\t</Project>\n");

  fileWrite("</CodeBlocks_project_file>\n");

  fileClose();
  return true;
}

void CodeBlocks::fileOpen(const String& name)
{
  if(!file.open(name, File::writeFlag))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
  openedFile = name;
}

void CodeBlocks::fileWrite(const String& data)
{
  if(!file.write(data))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
}

void CodeBlocks::fileClose()
{
  file.close();
  if(!openedFile.isEmpty())
  {
    puts(openedFile.getData());
    fflush(stdout);
  }
  openedFile.clear();
}

String CodeBlocks::join(const List<String>& items, char sep, const String& suffix)
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

String CodeBlocks::joinCommands(const List<String>& commands)
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

String CodeBlocks::xmlEscape(const String& text)
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
