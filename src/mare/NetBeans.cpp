
#include <cstdlib>
#include <cstdio>
//#include <cctype>

#include "Engine.h"

#include "Tools/Assert.h"
#include "Tools/Error.h"
//#include "Tools/Word.h"
#include "Tools/Directory.h"

#include "NetBeans.h"

bool NetBeans::generate(const Map<String, String>& userArgs)
{
  engine.addDefaultKey("tool", "NetBeans");
  engine.addDefaultKey("NetBeans", "NetBeans");
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

  // add user arguments
  for(const Map<String, String>::Node* i = userArgs.getFirst(); i; i = i->getNext())
    engine.addCommandLineKey(i->key, i->data);

  // step #1: read project data from marefile
  if(!readFile())
    return false;
    
  // step #2: ...
  if(!processData())
    return false;

  // step #3: generate project files
  for(Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
    if(!generateProject(i->data))
      return false;

  return true;
}

bool NetBeans::readFile()
{
  // get some global keys
  engine.enterRootKey();
  List<String> allPlatforms, allConfigurations, allTargets;
  engine.getKeys("platforms", allPlatforms);
  engine.getKeys("configurations", allConfigurations);
  engine.getKeys("targets", allTargets);
  engine.leaveKey();

  // do something for each target in each configuration
  for(const List<String>::Node* i = allPlatforms.getFirst(); i; i = 0) // just use the first platform
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
        engine.addDefaultKey("mareDir", engine.getMareDir());

        Map<String, Project>::Node* node = projects.find(i->data);
        Project& project = node ? node->data : projects.append(i->data, Project(i->data));

        Project::Config& projectConfig = project.configs.append(configName, Project::Config(configName));

        engine.getText("buildCommand", projectConfig.buildCommand, false);
        engine.getText("reBuildCommand", projectConfig.reBuildCommand, false);
        engine.getText("cleanCommand", projectConfig.cleanCommand, false);
        projectConfig.buildDir = engine.getFirstKey("buildDir", true);
        projectConfig.firstOutput = engine.getFirstKey("output", false);
        engine.getKeys("defines", projectConfig.defines, true);
        engine.getKeys("includePaths", projectConfig.includePaths, true);
        //engine.getKeys("libPaths", projectConfig.libPaths, true);
        //engine.getKeys("libs", projectConfig.libs, true);        

        List<String> dependencies;
        engine.getKeys("dependencies", dependencies, false);
        for(const List<String>::Node* i = dependencies.getFirst(); i; i = i->getNext())
          if(!project.dependencies.find(i->data))
            project.dependencies.append(i->data);

        List<String> root;
        engine.getKeys("root", root, true);
        for(const List<String>::Node* i = root.getFirst(); i; i = i->getNext())
          if(!project.roots.find(i->data))
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

bool NetBeans::processData()
{
  // remove projects not creating any output files
  for(Map<String, Project>::Node* i = projects.getFirst(), * nexti; i; i = nexti)
  {
    nexti = i->getNext();
    if(!i->data.files.isEmpty())
      continue;
    for(const Map<String, Project::Config>::Node* j = i->data.configs.getFirst(); j; j = j->getNext())
      if(!j->data.firstOutput.isEmpty())
        goto next;
    projects.remove(i);
  next:;
  }

  //
  if(projects.isEmpty())
  {
    engine.error("cannot find any targets");
    return false;
  }

  return true;
}

bool NetBeans::generateProject(Project& project)
{
  Directory::create(project.name);
  Directory::create(project.name + "/nbproject");
  
  fileOpen(project.name + "/nbproject/project.xml");
  fileWrite("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fileWrite("<project xmlns=\"http://www.netbeans.org/ns/project/1\">\n");
  fileWrite("    <type>org.netbeans.modules.cnd.makeproject</type>\n");
  fileWrite("    <configuration>\n");
  fileWrite("        <data xmlns=\"http://www.netbeans.org/ns/make-project/1\">\n");
  fileWrite(String("            <name>") + xmlEscape(project.name) + "</name>\n");
  fileWrite("            <c-extensions>c</c-extensions>\n");
  fileWrite("            <cpp-extensions>cc,cpp,mm</cpp-extensions>\n");
  fileWrite("            <header-extensions>h</header-extensions>\n");
  //fileWrite("            <sourceEncoding>UTF-8</sourceEncoding>\n");
  fileWrite("            <sourceEncoding>ISO-8859-1</sourceEncoding>\n");

  fileWrite("            <make-dep-projects>\n");
  for(const Map<String, void*>::Node* i = project.dependencies.getFirst(); i; i = i->getNext())
    fileWrite(String("                <make-dep-project>../") + xmlEscape(i->key) + "</make-dep-project>\n");
  fileWrite("            </make-dep-projects>\n");

  fileWrite("            <sourceRootList>\n");
  for(const Map<String, void*>::Node* i = project.roots.getFirst(); i; i = i->getNext())
    fileWrite(String("                <sourceRootElem>../") + xmlEscape(i->key) + "</sourceRootElem>\n");
  fileWrite("            </sourceRootList>\n");
  fileWrite("            <confList>\n");
  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    fileWrite("                <confElem>\n");
    fileWrite(String("                    <name>") + xmlEscape(i->key) + "</name>\n");
    fileWrite("                    <type>0</type>\n");
    fileWrite("                </confElem>\n");
  }
  fileWrite("            </confList>\n");
  fileWrite("        </data>\n");
  fileWrite("    </configuration>\n");
  fileWrite("</project>\n");
  fileClose();
  
  fileOpen(project.name + "/nbproject/configurations.xml");
  fileWrite("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  //fileWrite("<configurationDescriptor version=\"80\">\n");
  fileWrite("<configurationDescriptor version=\"84\">\n");
  fileWrite("  <logicalFolder name=\"root\" displayName=\"root\" projectFiles=\"true\" kind=\"ROOT\">\n");
  for(const Map<String, void*>::Node* i = project.roots.getFirst(); i; i = i->getNext())
  {
    fileWrite(String("    <df name=\"") + xmlEscape(File::getDirname(i->key)) + "\" root=\"../" + xmlEscape(i->key) + "\">\n");
    //fileWrite("      <df name=\"Icons\">\n");
    //fileWrite("      </df>\n");
    //fileWrite("      <in>MacStyle.h</in>\n");
    //fileWrite("      <in>MacStyle.mm</in>\n");
    //fileWrite("      <in>Main.cpp</in>\n");
    //fileWrite("      <in>MainWindow.cpp</in>\n");
    //fileWrite("      <in>MainWindow.h</in>\n");
    //fileWrite("      <in>RegisteredDockWidget.cpp</in>\n");
    //fileWrite("      <in>RegisteredDockWidget.h</in>\n");
    //fileWrite("      <in>SceneGraphDockWidget.cpp</in>\n");
    //fileWrite("      <in>SceneGraphDockWidget.h</in>\n");
    //fileWrite("      <in>SimRobot.h</in>\n");
    //fileWrite("      <in>StatusBar.cpp</in>\n");
    //fileWrite("      <in>StatusBar.h</in>\n");
    //fileWrite("      <in>qtdotnetstyle.cpp</in>\n");
    //fileWrite("      <in>qtdotnetstyle.h</in>\n");
    //fileWrite("      <in>resource.h</in>\n");
    fileWrite("    </df>\n");
  }
  fileWrite("  </logicalFolder>\n");
  fileWrite("  <sourceFolderFilter>^(nbproject)$</sourceFolderFilter>\n");
  fileWrite("  <sourceRootList>\n");
  for(const Map<String, void*>::Node* i = project.roots.getFirst(); i; i = i->getNext())
    fileWrite(String("    <Elem>../") + xmlEscape(i->key) + "</Elem>\n");;
  fileWrite("  </sourceRootList>\n");
  fileWrite("  <projectmakefile>Makefile</projectmakefile>\n");
  fileWrite("  <confs>\n");
  for(const Map<String, Project::Config>::Node* i = project.configs.getFirst(); i; i = i->getNext())
  {
    const Project::Config& projectConfig = i->data;
    fileWrite(String("    <conf name=\"") + xmlEscape(projectConfig.name) + "\" type=\"0\">\n");
    fileWrite("      <toolsSet>\n");
    fileWrite("        <remote-sources-mode>LOCAL_SOURCES</remote-sources-mode>\n");
    fileWrite("        <compilerSet>default</compilerSet>\n");
    fileWrite("      </toolsSet>\n");
    fileWrite("      <makefileType>\n");
    fileWrite("        <makeTool>\n");
    fileWrite("          <buildCommandWorkingDir>..</buildCommandWorkingDir>\n");
    fileWrite(String("          <buildCommand>") + joinCommands(projectConfig.buildCommand) + "</buildCommand>\n");
    fileWrite(String("          <cleanCommand>") + joinCommands(projectConfig.cleanCommand) + "</cleanCommand>\n");
    fileWrite(String("          <executablePath>../") + xmlEscape(projectConfig.firstOutput) + "</executablePath>\n");
    fileWrite("          <cTool>\n");
    fileWrite("            <incDir>\n");
    for(const List<String>::Node* i = projectConfig.includePaths.getFirst(); i; i = i->getNext())
      fileWrite(String("              <pElem>") + xmlEscape(i->data) + "</pElem>\n");
    fileWrite("            </incDir>\n");
    fileWrite("            <preprocessorList>\n");
    for(const List<String>::Node* i = projectConfig.defines.getFirst(); i; i = i->getNext())
      fileWrite(String("              <Elem>") + xmlEscape(i->data) + "</Elem>\n");
    fileWrite("            </preprocessorList>\n");
    fileWrite("          </cTool>\n");
    fileWrite("          <ccTool>\n");
    fileWrite("            <incDir>\n");
    for(const List<String>::Node* i = projectConfig.includePaths.getFirst(); i; i = i->getNext())
      fileWrite(String("              <pElem>") + xmlEscape(i->data) + "</pElem>\n");
    fileWrite("            </incDir>\n");
    fileWrite("            <preprocessorList>\n");
    for(const List<String>::Node* i = projectConfig.defines.getFirst(); i; i = i->getNext())
      fileWrite(String("              <Elem>") + xmlEscape(i->data) + "</Elem>\n");
    fileWrite("            </preprocessorList>\n");
    fileWrite("          </ccTool>\n");
    fileWrite("        </makeTool>\n");

    fileWrite("        <requiredProjects>\n");
    for(const Map<String, void*>::Node* i = project.dependencies.getFirst(); i; i = i->getNext())
    {
      const Map<String, Project>::Node* otherProjectNode = projects.find(i->key);
      const Map<String, Project::Config>::Node* otherProjectConfigNode = otherProjectNode ? otherProjectNode->data.configs.find(projectConfig.name) : 0;
      const Project::Config* otherProjectConfig = otherProjectConfigNode ? &otherProjectConfigNode->data : 0;
      if(!otherProjectConfig)
        continue;
      fileWrite(String("          <makeArtifact PL=\"../") + xmlEscape(i->key) + "\"\n");
      fileWrite("                        CT=\"0\"\n");
      fileWrite(String("                        CN=\"") + xmlEscape(projectConfig.name) + "\"\n");
      fileWrite("                        AC=\"true\"\n");
      fileWrite("                        BL=\"false\"\n");
      fileWrite("                        WD=\"..\"\n");
      fileWrite(String("                        BC=\"") + joinCommands(otherProjectConfig->buildCommand) + "\"\n");
      fileWrite(String("                        CC=\"") + joinCommands(otherProjectConfig->cleanCommand) + "\"\n");
      fileWrite(String("                        OP=\"") + xmlEscape(otherProjectConfig->firstOutput) + "\">\n");
      fileWrite("          </makeArtifact>\n");
    }
    fileWrite("        </requiredProjects>\n");

    fileWrite("      </makefileType>\n");
    fileWrite("    </conf>\n");
  }
  fileWrite("  </confs>\n");
  fileWrite("</configurationDescriptor>\n");
  fileClose();
  return true;
}

void NetBeans::fileOpen(const String& name)
{
  if(!file.open(name, File::writeFlag))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
  openedFile = name;
}

void NetBeans::fileWrite(const String& data)
{
  if(!file.write(data))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
}

void NetBeans::fileClose()
{
  file.close();
  if(!openedFile.isEmpty())
  {
    puts(openedFile.getData());
    fflush(stdout);
  }
  openedFile.clear();
}
/*
String NetBeans::join(const List<String>& items, char sep, const String& suffix)
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
*/
String NetBeans::joinCommands(const List<String>& commands)
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

String NetBeans::xmlEscape(const String& text)
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
