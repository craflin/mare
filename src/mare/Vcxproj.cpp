
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "../libmare/Tools/File.h"
#include "Tools/md5.h"
#include "../libmare/Engine.h"

#include "Vcxproj.h"

bool Vcxproj::generate(const Map<String, String>& userArgs)
{
  engine.addDefaultKey("platforms", "Win32");
  engine.enterDefaultKey("configurations");
    engine.addResolvableKey("Debug");
    engine.addResolvableKey("Release");
  engine.leaveKey();
  engine.addDefaultKey("targets");
  engine.addDefaultKey("buildDir", "$(configuration)");
  engine.enterDefaultKey("cppCompile");
    engine.addResolvableKey("command", "__clCompile");
  engine.leaveKey();
  engine.enterDefaultKey("cppLink");
    engine.addResolvableKey("command", "__Application");
  engine.leaveKey();

  // add user arguments
  engine.enterUnnamedKey();
  for(const Map<String, String>::Node* i = userArgs.getFirst(); i; i = i->getNext())
    engine.addDefaultKey(i->key, i->data);

  // get some global keys
  solutionName = engine.getFirstKey("name");
  engine.getKeys("platforms", platforms);
  List<String> target;
  engine.getKeys("target", target);
  for(const List<String>::Node* i = target.getFirst(); i; i = i->getNext())
    activesProjects.append(i->data, 0);

  // enter configurations space
  engine.enterKey("configurations");

  // get configuration project list
  engine.getKeys(configurations);
  for(const List<String>::Node* i = configurations.getFirst(); i; i = i->getNext())
  {
    const String& configuration = i->data;
    engine.enterKey(configuration);

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
      project->configurations.append(configuration, 0);
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

  return true;
}

bool Vcxproj::generateSln()
{
  fileOpen(solutionName + ".sln");

  // header
  fileWrite("\r\n");
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
  for(const List<String>::Node* i = platforms.getFirst(); i; i = i->getNext())
  {
    const String& platform = i->data;
    for(const List<String>::Node* i = configurations.getFirst(); i; i = i->getNext())
    {
      const String& config = i->data;
      fileWrite(String("\t\t") + config + "|" + platform + " = " + config + "|" + platform + "\r\n");
    }
  }
  fileWrite("\tEndGlobalSection\r\n");

  // solution config to project config map
  fileWrite("\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\r\n");
  for(const Map<String, Project>::Node* i = projects.getFirst(); i; i = i->getNext())
  {
    const Project& project = i->data;
    for(const List<String>::Node* i = platforms.getFirst(); i; i = i->getNext())
    {
      const String& platform = i->data;
      for(const List<String>::Node* i = configurations.getFirst(); i; i = i->getNext())
      {
        const String& config = i->data;
        const String& projectConfig = project.configurations.find(config) ? config : project.configurations.getFirst()->key;
        fileWrite(String("\t\t{") + project.guid + "}." + config + "|" + platform + ".ActiveCfg = " + projectConfig + "|" + platform + "\r\n");
        if(activesProjects.isEmpty() || activesProjects.find(project.name))
          fileWrite(String("\t\t{") + project.guid + "}." + config + "|" + platform + ".Build.0 = " + projectConfig + "|" + platform + "\r\n");
      }
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

  return true;
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

void Vcxproj::fileOpen(const String& name)
{
  if(!file.open(name, File::writeFlag))
  {
    fprintf(stderr, "%s\n", file.getErrno().getString().getData());
    exit(EXIT_FAILURE);
  }
}

void Vcxproj::fileWrite(const String& data)
{
  if(!file.write(data))
  {
    fprintf(stderr, "%s\n", file.getErrno().getString().getData());
    exit(EXIT_FAILURE);
  }
}
