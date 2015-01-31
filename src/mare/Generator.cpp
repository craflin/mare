
#include <cstdlib>
#include <cstdio>
#include <cctype>

#include "Engine.h"

#include "Tools/Assert.h"
#include "Tools/Error.h"
#include "Tools/Word.h"

#include "Generator.h"

bool Generator::generate(const Map<String, String>& userArgs)
{
  addDefaultKeys(engine);

  // add user arguments
  for(const Map<String, String>::Node* i = userArgs.getFirst(); i; i = i->getNext())
    engine.addCommandLineKey(i->key, i->data);

  // step #1: read input file
  Data data;
  if(!readFile(data))
    return false;

  // step #2: ...
  if(!processData(data))
    return false;

  // step #3:  generate workspace and project files
  if(!writeFiles())
    return false;

  return true;
}

bool Generator::readFile(Data& data)
{
  // get some global keys
  engine.enterRootKey();
  data.name = engine.getFirstKey("name");
  List<String> allPlatforms, allConfigurations, allTargets;
  engine.getKeys("platforms", allPlatforms);
  engine.getKeys("configurations", allConfigurations);
  engine.getKeys("targets", allTargets);
  engine.leaveKey();

  // do something for each target in each configuration
  for(const List<String>::Node* i = allPlatforms.getFirst(); i; i = 0) // just use the first platform since Generator does not really support multiple target platforms
  {
    const String& platformName = i->data;
    Platform& platform = data.platforms.append(platformName);
    
    for(const List<String>::Node* i = allConfigurations.getFirst(); i; i = i->getNext())
    {
      const String& configName = i->data;
      Configuration& configuration = platform.configurations.append(configName);

      for(const List<String>::Node* i = allTargets.getFirst(); i; i = i->getNext())
      {
        const String& targetName = i->data;

        engine.enterUnnamedKey();
        engine.addDefaultKey("platform", platformName);
        engine.addDefaultKey(platformName, platformName);
        engine.addDefaultKey("configuration", configName);
        engine.addDefaultKey(configName, configName);
        engine.addDefaultKey("target", targetName);
        //engine.addDefaultKey(targetName, targetName);
        engine.enterRootKey();
        VERIFY(engine.enterKey("targets"));
        if(!engine.enterKey(i->data))
        {
          engine.error(String().format(256, "cannot find target \"%s\"", i->data.getData()));
          return false;
        }
        engine.addDefaultKey("mareDir", engine.getMareDir());

        Target& target = configuration.targets.append(targetName);

        target.folder = engine.getFirstKey("folder", false);

        engine.getText("buildCommand", target.buildCommand, false);
        engine.getText("reBuildCommand", target.reBuildCommand, false);
        engine.getText("cleanCommand", target.cleanCommand, false);

        engine.getText("preBuildCommand", target.preBuildCommand, false);
        engine.getText("preLinkCommand", target.preLinkCommand, false);
        engine.getText("postBuildCommand", target.postBuildCommand, false);

        target.buildDir = engine.getFirstKey("buildDir", true);

        engine.getText("command", target.command, false);
        engine.getText("message", target.message, false);
        engine.getKeys("output", target.output, false);
        engine.getKeys("input", target.input, false);

        engine.getKeys("dependencies", target.dependencies, false);

        engine.getKeys("cppFlags", target.cppFlags, true);
        engine.getKeys("cFlags", target.cFlags, true);
        engine.getKeys("linkFlags", target.linkFlags, true);
        engine.getKeys("defines", target.defines, true);
        engine.getKeys("includePaths", target.includePaths, true);
        engine.getKeys("libPaths", target.libPaths, true);
        engine.getKeys("libs", target.libs, true);

        engine.getKeys("root", target.root, true);

        if(engine.enterKey("files"))
        {
          List<String> files;
          engine.getKeys(files);
          for(const List<String>::Node* i = files.getFirst(); i; i = i->getNext())
          {
            const String& fileName = i->data;
            File& file = target.files.append(fileName);
            engine.enterUnnamedKey();
            engine.addDefaultKey("file", fileName);
            VERIFY(engine.enterKey(fileName));
            file.folder = engine.getFirstKey("folder", false);

            engine.getText("command", file.command, false);
            engine.getText("message", file.message, false);
            engine.getKeys("output", file.output, false);
            engine.getKeys("input", file.input, false);
            engine.getKeys("dependencies", file.dependencies, false);
            file.hasCppFlags = engine.getKeys("cppFlags", file.cppFlags, false);
            file.hasCFlags = engine.getKeys("cFlags", file.cFlags, false);

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

void Generator::fileOpen(const String& name)
{
  if(!file.open(name, ::File::writeFlag))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
  openedFile = name;
}

void Generator::fileWrite(const String& data)
{
  if(!file.write(data))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
}

void Generator::fileClose()
{
  file.close();
  if(!openedFile.isEmpty())
  {
    puts(openedFile.getData());
    fflush(stdout);
  }
  openedFile.clear();
}

void Generator::error(const String& message)
{
  engine.error(message);
}
