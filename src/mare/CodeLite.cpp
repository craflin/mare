
#include <cstdlib>
#include <cstdio>
#include <cctype>

#include "Engine.h"

#include "Tools/Assert.h"
#include "Tools/Error.h"

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
  engine.addDefaultKey("targets");
  engine.addDefaultKey("buildDir", "$(configuration)");

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
  workspaceName = engine.getFirstKey("name");

  List<String> inputPlatforms, inputConfigurations, inputTargets;
  engine.getKeys("platforms", inputPlatforms);
  engine.getKeys("configurations", inputConfigurations);
  engine.getKeys("targets", inputTargets);

  engine.enterKey("platforms");

  for(const List<String>::Node* i = inputPlatforms.getFirst(); i; i = 0) // just use the first platform since CodeLite does not really support multiple target platforms
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
      configs.append(configName);

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
        Project& project = node ? node->data : projects.append(i->data, Project(i->data));
        //bool isNewProject = node && true;

        Project::Config& projectConfig = project.configs.append(configName, Project::Config(configName));

        if(!engine.enterKey(i->data))
        {
          engine.error(String().format(256, "cannot find target \"%s\"", i->data.getData()));
          return false;
        }
        engine.addDefaultKey("target", i->data);

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
        
        engine.getKeys("command", projectConfig.command, false);
        projectConfig.firstOutput = engine.getFirstKey("outputs", false);

        projectConfig.type = "Utility";
        if(!projectConfig.command.isEmpty())
        {
          const String& firstCommand = projectConfig.command.getFirst()->data;
          if(firstCommand == "__Custom")
          {
            projectConfig.customBuild = true;
            if(projectConfig.command.getSize() > 1)
            {
              const String& nextCommand = projectConfig.command.getFirst()->getNext()->data;
              if(nextCommand == "Application" || nextCommand == "StaticLibrary" || nextCommand == "DynamicLibrary")
                projectConfig.type = nextCommand;
            }
            projectConfig.command.clear();
          }
          else if(firstCommand == "__Application" || firstCommand == "__StaticLibrary" || firstCommand == "__DynamicLibrary")
          {
            projectConfig.type = firstCommand.substr(2);
            projectConfig.command.clear();
          }
        }

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
            Map<String, void*>::Node* node = project.files.find(i->data);
            if(!node)
              project.files.append(i->data);
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

bool CodeLite::generateWorkspace()
{
  // create solution file name
  if(workspaceName.isEmpty() && !projects.isEmpty())
    workspaceName = projects.getFirst()->data.name;

  // open output file
  fileOpen(workspaceName + ".workspace");

  // write
  fileWrite("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
  fileWrite(String("<CodeLite_Workspace Name=\"") + workspaceName+ "\" Database=\"./" + workspaceName + ".tags\">\n");
  /*
  fileWrite("  <Project Name=\"myproject\" Path=\"myproject/myproject.project\" Active=\"Yes\"/>\n");
  fileWrite("  <Project Name=\"project2\" Path=\"project2/project2.project\"/>\n");
  fileWrite("  <BuildMatrix>\n");
  fileWrite("    <WorkspaceConfiguration Name=\"Debug\" Selected=\"yes\">\n");
  fileWrite("      <Project Name=\"myproject\" ConfigName=\"Debug\"/>\n");
  fileWrite("      <Project Name=\"project2\" ConfigName=\"Debug\"/>\n");
  fileWrite("    </WorkspaceConfiguration>\n");
  fileWrite("    <WorkspaceConfiguration Name=\"Release\" Selected=\"yes\">\n");
  fileWrite("      <Project Name=\"myproject\" ConfigName=\"Release\"/>\n");
  fileWrite("      <Project Name=\"project2\" ConfigName=\"Release\"/>\n");
  fileWrite("    </WorkspaceConfiguration>\n");
  fileWrite("  </BuildMatrix>\n");
  */
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

  // ...

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
  String result;
  const List<String>::Node* i = commands.getFirst();
  if(i)
  {
    String program(i->data);
    program.subst("/", "\\");
    for(const char* str = program.getData(); *str; ++str)
        if(isspace(*str))
        {
          result.append('"');
          result.append(xmlEscape(program));
          result.append('"');
          goto next;
        }
      result.append(xmlEscape(program));
    next: ;
    for(i = i->getNext(); i; i = i->getNext())
    {
      result.append(' ');
      for(const char* str = i->data.getData(); *str; ++str)
        if(isspace(*str))
        {
          result.append('"');
          result.append(xmlEscape(i->data));
          result.append('"');
          goto next2;
        }
      result.append(xmlEscape(i->data));
    next2: ;
    }
  }
  return result;
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
