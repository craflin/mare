
#include <cstdlib>
#include <cstdio>

#include "Engine.h"
#include "Tools/Assert.h"
#include "Tools/Error.h"
#include "Tools/Word.h"
#include "Tools/Directory.h"

#include "Make.h"

bool Make::generate(const Map<String, String>& userArgs)
{
  // add default keys
  engine.addDefaultKey("tool", "Make");
  engine.addDefaultKey("Make", "Make");
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
    Map<String, String> cppApplication;
    cppApplication.append("input", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    cppApplication.append("output", "$(buildDir)/$(target)$(if $(Win32),.exe)");
    cppApplication.append("command", "$(CXX) -o $(output) $(input) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    cppApplication.append("message", "Linking $(target)...");
    engine.addDefaultKey("cppApplication", cppApplication);
  }
  {
    Map<String, String> cppDynamicLibrary;
    cppDynamicLibrary.append("input", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    cppDynamicLibrary.append("output", "$(buildDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.dll,.so)");
    cppDynamicLibrary.append("__soFlags", "$(if $(Win32),,-fpic)");
    cppDynamicLibrary.append("command", "$(CXX) -shared $(__soFlags) -o $(output) $(input) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    cppDynamicLibrary.append("message", "Linking $(target)...");
    engine.addDefaultKey("cppDynamicLibrary", cppDynamicLibrary);
  }
  {
    Map<String, String> cppStaticLibrary;
    cppStaticLibrary.append("input", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    cppStaticLibrary.append("output", "$(buildDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.lib,.a)");
    cppStaticLibrary.append("command", "$(AR) rcs $(output) $(input)");
    cppStaticLibrary.append("message", "Creating $(target)...");
    engine.addDefaultKey("cppStaticLibrary", cppStaticLibrary);
  }
  {
    Map<String, String> cApplication;
    cApplication.append("input", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    cApplication.append("output", "$(buildDir)/$(target)$(if $(Win32),.exe)");
    cApplication.append("command", "$(CC) -o $(output) $(input) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    cApplication.append("message", "Linking $(target)...");
    engine.addDefaultKey("cApplication", cApplication);
  }
  {
    Map<String, String> cDynamicLibrary;
    cDynamicLibrary.append("input", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    cDynamicLibrary.append("output", "$(buildDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.dll,.so)");
    cDynamicLibrary.append("__soFlags", "$(if $(Win32),,-fpic)");
    cDynamicLibrary.append("command", "$(CC) -shared $(__soFlags) -o $(output) $(input) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    cDynamicLibrary.append("message", "Linking $(target)...");
    engine.addDefaultKey("cDynamicLibrary", cDynamicLibrary);
  }
  {
    Map<String, String> cStaticLibrary;
    cStaticLibrary.append("input", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    cStaticLibrary.append("output", "$(buildDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.lib,.a)");
    cStaticLibrary.append("command", "$(AR) rcs $(output) $(input)");
    cStaticLibrary.append("message", "Creating $(target)...");
    engine.addDefaultKey("cStaticLibrary", cStaticLibrary);
  }
  {
    Map<String, String> cppSource;
    cppSource.append("__ofile", "$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file)))");
    cppSource.append("input", "$(file)");
    cppSource.append("output", "$(__ofile)");
    cppSource.append("command", "$(CXX) -MMD $(__soFlags) -o $(__ofile) -c $(file) $(cppFlags) $(CXXFLAGS) $(patsubst %,-D%,$(defines)) $(patsubst %,-I%,$(includePaths))");
    cppSource.append("message", "$(file)");
    engine.addDefaultKey("cppSource", cppSource);
  }
  {
    Map<String, String> cSource;
    cSource.append("__ofile", "$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file)))");
    cSource.append("input", "$(file)");
    cSource.append("output", "$(__ofile)");
    cSource.append("command", "$(CC) -MMD $(__soFlags) -o $(__ofile) -c $(file) $(cFlags) $(CFLAGS) $(patsubst %,-D%,$(defines)) $(patsubst %,-I%,$(includePaths))");
    cSource.append("message", "$(file)");
    engine.addDefaultKey("cSource", cSource);
  }

  // add user arguments
  for(const Map<String, String>::Node* i = userArgs.getFirst(); i; i = i->getNext())
    engine.addDefaultKey(i->key, i->data);

  // step #1: read input file
  if(!readFile())
    return false;

  // step #2 ...
  if(!processData())
    return false;

  // step #3: generate output files
  if(!generateMakefile())
    return false;

  return true;
}

bool Make::readFile()
{
  // get some global keys
  engine.enterRootKey();
  List<String> allPlatforms, allConfigurations, allTargets;
  engine.getKeys("platforms", allPlatforms);
  engine.getKeys("configurations", allConfigurations);
  engine.getKeys("targets", allTargets);

  // do something for each target in each configuration
  for(const List<String>::Node* i = allPlatforms.getFirst(); i; i = i->getNext())
  {
    const String& platformName = i->data;
    Platform& platform = platforms.append(Platform(platformName));
    for(const List<String>::Node* i = allConfigurations.getFirst(); i; i = i->getNext())
    {
      const String& configName = i->data;
      Platform::Config& config = platform.configs.append(Platform::Config(configName));

      for(const List<String>::Node* i = allTargets.getFirst(); i; i = i->getNext())
      {
        const String& targetName = i->data;

        engine.enterUnnamedKey();
        engine.addDefaultKey("platform", platformName);
        engine.addDefaultKey(platformName, platformName);
        engine.addDefaultKey("configuration", configName);
        engine.addDefaultKey(configName, configName);
        engine.addDefaultKey("target", targetName);

        engine.enterRootKey();
        VERIFY(engine.enterKey("targets"));
        if(!engine.enterKey(targetName))
        {
          engine.error(String().format(256, "cannot find target \"%s\"", targetName.getData()));
          return false;
        }

        Platform::Config::Target& target = config.targets.append(Platform::Config::Target(targetName));

        engine.getText("command", target.command, false);
        engine.getText("message", target.message, false);
        engine.getKeys("dependencies", target.dependencies, false);
        engine.getKeys("output", target.output, false);
        engine.getKeys("input", target.input, false);

        if(engine.enterKey("files"))
        {
          List<String> files;
          engine.getKeys(files);
          for(const List<String>::Node* i = files.getFirst(); i; i = i->getNext())
          {
            Platform::Config::Target::File& file = target.files.append();

            engine.enterUnnamedKey();
            engine.addDefaultKey("file", i->data);
            VERIFY(engine.enterKey(i->data));

            engine.getText("command", file.command, false);
            engine.getText("message", file.message, false);
            engine.getKeys("dependencies", file.dependencies, false);
            engine.getKeys("input", file.input, false);
            engine.getKeys("output", file.output, false);

            engine.leaveKey();
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

bool Make::processData()
{
  for(List<Platform>::Node* k = platforms.getLast(); k; k = k->getPrevious())
    for(List<Platform::Config>::Node* j = k->data.configs.getLast(); j; j = j->getPrevious())
    {
      Platform::Config& config = j->data;
      for(List<Platform::Config::Target>::Node* i = config.targets.getFirst(); i; i = i->getNext())
      {
        const Platform::Config::Target& target = i->data;

        for(const List<String>::Node* i = target.output.getFirst(); i; i = i->getNext())
        {
          String outputDir = File::getDirname(i->data);
          if(outputDir != "." && !config.outputDirs.find(outputDir))
            config.outputDirs.append(outputDir);
        }

        for(const List<Platform::Config::Target::File>::Node* j = target.files.getFirst(); j; j = j->getNext())
          for(const List<String>::Node* i = j->data.output.getFirst(); i; i = i->getNext())
          {
            String outputDir = File::getDirname(i->data);
            if(outputDir != "." && !config.outputDirs.find(outputDir))
              config.outputDirs.append(outputDir);
          }
      }
    }
  return true;
}

bool Make::generateMakefile()
{
  fileOpen("Makefile");
  fileWrite("\n");

  fileWrite(".SUFFIXES:\n"); // disable some built-in rules
  fileWrite("\n");

  if(platforms.getSize() == 1)
    generateMakefilePlatform(platforms.getFirst()->data);
  else if(platforms.getSize() > 1)
  {
    for(List<Platform>::Node* j = platforms.getLast(); j; j = j->getPrevious())
    {
      Platform& platform = j->data;
      if(j == platforms.getLast())
        fileWrite(String("ifeq ($(platform),") + platform.name + ")\n");
      else 
        fileWrite(String("else ifeq ($(platform),") + platform.name + ")\n");
      fileWrite("\n");
      generateMakefilePlatform(platform);
    }
    fileWrite("endif\n");
  }

  fileWrite("\n");
  fileClose();
  return true;
}

void Make::generateMakefilePlatform(Platform& platform)
{
  if(platform.configs.getSize() == 1)
    generateMakefileConfig(platform.configs.getFirst()->data);
  else if(platform.configs.getSize() > 1)
  {
    for(List<Platform::Config>::Node* j = platform.configs.getLast(); j; j = j->getPrevious())
    {
      Platform::Config& config = j->data;
      if(j == platform.configs.getLast())
        fileWrite(String("ifeq ($(config),") + config.name + ")\n");
      else 
        fileWrite(String("else ifeq ($(config),") + config.name + ")\n");
      fileWrite("\n");
      generateMakefileConfig(config);
    }
    fileWrite("endif\n");
    fileWrite("\n");
  }
}

void Make::generateMakefileConfig(Platform::Config& config)
{
  for(const List<Platform::Config::Target>::Node* i = config.targets.getFirst(); i; i = i->getNext())
  {
    const Platform::Config::Target& target = i->data;

    if(target.output.isEmpty() || target.name != target.output.getFirst()->data)
    {
      fileWrite(String(".PHONY: ") + target.name + "\n");
      if(target.output.isEmpty())
        fileWrite(target.name + ": " + join(target.dependencies) + "\n");
      else
        fileWrite(target.name + ": " + target.output.getFirst()->data + " " + join(target.dependencies) + "\n");
      fileWrite("\n");
    }

    if(!target.output.isEmpty())
    {
      const String& outputFile = target.output.getFirst()->data;
      fileWrite(outputFile + ": " + join(target.input) + " " + join(target.dependencies) + " | \n");
      bool hasMessage = !target.message.isEmpty();
      if(hasMessage)
        fileWrite(joinCommands("\t@echo \"", "\"\n", target.message));
      if(!target.command.isEmpty())
        fileWrite(joinCommands(hasMessage ? String("\t@") : String("\t"), "\n", target.command));
      fileWrite("\n");

      for(const List<String>::Node* i = target.output.getFirst()->getNext(); i; i = i->getNext())
        fileWrite(i->data + ": " + outputFile + "\n");
    }

    for(const List<Platform::Config::Target::File>::Node* i = target.files.getFirst(); i; i = i->getNext())
    {
      const Platform::Config::Target::File& file = i->data;

      if(!file.output.isEmpty())
      {
        const String& outputFile = file.output.getFirst()->data;
        fileWrite(outputFile + ": " + join(file.input) + " " + join(file.dependencies) + " | \n");
        bool hasMessage = !file.message.isEmpty();
        if(hasMessage)
          fileWrite(joinCommands("\t@echo \"", "\"\n", file.message));
        if(!file.command.isEmpty())
          fileWrite(joinCommands(hasMessage ? String("\t@") : String("\t"), "\n", file.command));
        fileWrite("\n");

        for(const List<String>::Node* i = file.output.getFirst()->getNext(); i; i = i->getNext())
          fileWrite(i->data + ": " + outputFile + "\n");
      }
    }
  }
}

void Make::fileOpen(const String& name)
{
  if(!file.open(name, File::writeFlag))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
  openedFile = name;
}

void Make::fileWrite(const String& data)
{
  if(!file.write(data))
  {
    engine.error(Error::getString());
    exit(EXIT_FAILURE);
  }
}

void Make::fileClose()
{
  file.close();
  if(!openedFile.isEmpty())
  {
    puts(openedFile.getData());
    fflush(stdout);
  }
  openedFile.clear();
}

String Make::join(const List<String>& items)
{
  String result;
  const List<String>::Node* i = items.getFirst();
  if(i)
  {
    result = i->data;
    for(i = i->getNext(); i; i = i->getNext())
    {
      result.append(' ');
      result.append(i->data);
    }
  }
  return result;
}

String Make::joinCommands(const String& prefix, const String& suffix, const List<String>& commands)
{
  String result;
  for(const List<String>::Node* i = commands.getFirst(); i; i = i->getNext())
    result += prefix + i->data + suffix;
  return result;
}
