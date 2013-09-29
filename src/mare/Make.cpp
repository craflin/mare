
#include <cstdlib>
#include <cstdio>
#include <cstring>

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
    Map<String, String> cApplication;
    cApplication.append("command", "__cApplication");
    cApplication.append("output", "$(buildDir)/$(target)$(if $(Win32),.exe)");
    engine.addDefaultKey("cApplication", cApplication);
  }
  {
    Map<String, String> cppApplication;
    cppApplication.append("command", "__cppApplication");
    cppApplication.append("output", "$(buildDir)/$(target)$(if $(Win32),.exe)");
    engine.addDefaultKey("cppApplication", cppApplication);
  }
  {
    Map<String, String> cDynamicLibrary;
    cDynamicLibrary.append("command", "__cDynamicLibrary");
    cDynamicLibrary.append("output", "$(buildDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.dll,.so)");
    cDynamicLibrary.append("__soFlags", "$(if $(Win32),,-fpic)");
    engine.addDefaultKey("cDynamicLibrary", cDynamicLibrary);
  }
  {
    Map<String, String> cppDynamicLibrary;
    cppDynamicLibrary.append("command", "__cppDynamicLibrary");
    cppDynamicLibrary.append("output", "$(buildDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.dll,.so)");
    cppDynamicLibrary.append("__soFlags", "$(if $(Win32),,-fpic)");
    engine.addDefaultKey("cppDynamicLibrary", cppDynamicLibrary);
  }
  {
    Map<String, String> cStaticLibrary;
    cStaticLibrary.append("command", "__cStaticLibrary");
    cStaticLibrary.append("output", "$(buildDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.lib,.a)");
    engine.addDefaultKey("cStaticLibrary", cStaticLibrary);
  }
  {
    Map<String, String> cppStaticLibrary;
    cppStaticLibrary.append("command", "__cppStaticLibrary");
    cppStaticLibrary.append("output", "$(buildDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.lib,.a)");
    engine.addDefaultKey("cppStaticLibrary", cppStaticLibrary);
  }

  // add user arguments
  for(const Map<String, String>::Node* i = userArgs.getFirst(); i; i = i->getNext())
    engine.addCommandLineKey(i->key, i->data);

  // step #1: read input file
  if(!readFile())
    return false;

  // step #2 ...
  if(!processData())
    return false;

  // step #3: generate output files
  if(!generateMetaMakefile())
    return false;
  for(const List<Platform>::Node* k = platforms.getFirst(); k; k = k->getNext())
    for(const List<Platform::Config>::Node* j = k->data.configs.getFirst(); j; j = j->getNext())
      if(!generateMakefile(k->data, j->data))
        return false;
  for(const List<Platform>::Node* k = platforms.getFirst(); k; k = k->getNext())
    for(const List<Platform::Config>::Node* j = k->data.configs.getFirst(); j; j = j->getNext())
      for(const Map<String, Platform::Config::Target>::Node* i = j->data.targets.getFirst(); i; i = i->getNext())
      {
        const Platform::Config::Target& target = i->data;
        if(!target.files.isEmpty() || !target.command.isEmpty() || !target.output.isEmpty() || !target.type.isEmpty())
          if(!generateTargetMakefile(k->data, j->data, target))
            return false;
      }

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

        Platform::Config::Target& target = config.targets.append(targetName, Platform::Config::Target(targetName));

        engine.getText("command", target.command, false);
        engine.getText("message", target.message, false);
        engine.getKeys("dependencies", target.dependencies, false);
        engine.getKeys("output", target.output, false);
        engine.getKeys("input", target.input, false);

        target.buildDir = engine.getFirstKey("buildDir", true);
        engine.getKeys("__soFlags", target.soFlags, true);
        engine.getKeys("cppFlags", target.cppFlags, true);
        engine.getKeys("cFlags", target.cFlags, true);
        engine.getKeys("linkFlags", target.linkFlags, true);
        engine.getKeys("defines", target.defines, true);
        engine.getKeys("includePaths", target.includePaths, true);
        engine.getKeys("libPaths", target.libPaths, true);
        engine.getKeys("libs", target.libs, true);

        engine.getKeys("cppCompiler", target.cppCompiler, true);
        engine.getKeys("cCompiler", target.cCompiler, true);
        engine.getKeys("linker", target.linker, true);

        if(engine.enterKey("files"))
        {
          List<String> files;
          engine.getKeys(files);
          for(const List<String>::Node* i = files.getFirst(); i; i = i->getNext())
          {
            Platform::Config::Target::File& file = target.files.append(Platform::Config::Target::File(i->data));

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

      // handle {c,cpp}{Application,DynamicLibrary,StaticLibrary,Source} rules
      for(Map<String, Platform::Config::Target>::Node* i = config.targets.getFirst(); i; i = i->getNext())
      {
        Platform::Config::Target& target = i->data;
        String firstCommand;
        if(!target.command.isEmpty())
          firstCommand = target.command.getFirst()->data;

        if(firstCommand  == "__cApplication" || firstCommand  == "__cppApplication" || 
           firstCommand  == "__cDynamicLibrary" || firstCommand  == "__cppDynamicLibrary" ||
           firstCommand  == "__cStaticLibrary" || firstCommand  == "__cppStaticLibrary")
        {
          target.type = firstCommand;
          target.command.clear();
          target.message.clear();
          String linker = !strncmp(firstCommand.getData(), "__cpp", 5) ? String("$(__CXXLINKER)") : String("$(__CCLINKER)");
          if(firstCommand  == "__cApplication" || firstCommand  == "__cppApplication")
          {
            target.command.append(linker + " -o $@ $(__OBJECTS) $(__LINKFLAGS) $(LDFLAGS) $(__LIBPATHS) $(__LIBS)");
            target.message.append(String("Linking ") + target.name + "...");
          }
          else if(firstCommand  == "__cDynamicLibrary" || firstCommand  == "__cppDynamicLibrary")
          {
            target.command.append(linker + " -shared $(__SOFLAGS) -o $@ $(__OBJECTS) $(__LINKFLAGS) $(LDFLAGS) $(__LIBPATHS) $(__LIBS)");
            target.message.append(String("Linking ") + target.name + "...");
          }
          else if(firstCommand  == "__cStaticLibrary" || firstCommand  == "__cppStaticLibrary")
          {
            target.command.append("$(__ARLINKER) rcs $@ $(__OBJECTS)");
            target.message.append(String("Creating ") + target.name + "...");
          }
        }

        for(List<Platform::Config::Target::File>::Node* j = target.files.getFirst(); j; j = j->getNext())
        {
          Platform::Config::Target::File& file = j->data;
          String firstCommand;
          if(!file.command.isEmpty())
            firstCommand = file.command.getFirst()->data;

          if(firstCommand  == "__cSource" || firstCommand  == "__cppSource")
          {
            file.type = firstCommand;
            file.command.clear();
            file.output.clear();
            file.message.clear();
            String fileName = file.name;
            fileName.subst("../", "");
            fileName = target.buildDir + "/" + File::getWithoutExtension(fileName) + ".o";
            file.output.append(fileName);
            target.objects.append(fileName);
            file.input.prepend(file.name);
            String compiler = firstCommand  == "__cppSource" ? String("$(CXX)") : String("$(CC)");
            if(firstCommand  == "__cSource")
              file.command.append(compiler + " -MMD $(__SOFLAGS) -o $@ -c $< $(__CFLAGS) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(__DEFINES) $(__INCLUDEPATHS)");
            else
              file.command.append(compiler + " -MMD $(__SOFLAGS) -o $@ -c $< $(__CPPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(__DEFINES) $(__INCLUDEPATHS)");
            file.message.append(file.name);
          }
          else if(!file.output.isEmpty())
            target.nonObjects.append(file.output.getFirst()->data);
        }
      }

      // generate outputDirs list
      for(Map<String, Platform::Config::Target>::Node* i = config.targets.getFirst(); i; i = i->getNext())
      {
        Platform::Config::Target& target = i->data;

        for(const List<String>::Node* i = target.output.getFirst(); i; i = i->getNext())
        {
          String outputDir = File::getDirname(i->data);
          if(outputDir != "." && !target.outputDirs.find(outputDir))
            target.outputDirs.append(outputDir);
        }

        for(List<Platform::Config::Target::File>::Node* j = target.files.getFirst(); j; j = j->getNext())
          for(const List<String>::Node* i = j->data.output.getFirst(); i; i = i->getNext())
          {
            String outputDir = File::getDirname(i->data);
            if(outputDir != "." && !target.outputDirs.find(outputDir))
              target.outputDirs.append(outputDir);
          }
      }

      // convert dependencies into additional input files
      for(Map<String, Platform::Config::Target>::Node* i = config.targets.getFirst(); i; i = i->getNext())
      {
        Platform::Config::Target& target = i->data;
        for(const List<String>::Node* i = target.dependencies.getFirst(); i; i = i->getNext())
        {
          const Map<String, Platform::Config::Target>::Node* node = config.targets.find(i->data);
          if(!node)
            printf("warning: Cannot resolve dependency \"%s\"\n", i->data.getData());
          else if(&node->data == &target)
            continue;
          else
            for(const List<String>::Node* j = node->data.output.getFirst(); j; j = j->getNext())
              target.input.append(j->data);
        }
        for(List<Platform::Config::Target::File>::Node* j = target.files.getFirst(); j; j = j->getNext())
        {
          Platform::Config::Target::File& file = j->data;
          for(const List<String>::Node* i = file.dependencies.getFirst(); i; i = i->getNext())
          {
            const Map<String, Platform::Config::Target>::Node* node = config.targets.find(i->data);
            if(!node)
              printf("warning: Cannot resolve dependency \"%s\"\n", i->data.getData());
            else if(&node->data == &target)
              continue;
            else
              for(const List<String>::Node* j = node->data.output.getFirst(); j; j = j->getNext())
                file.input.append(j->data);
          }
        }
      }

      // add file dependencies to target dependencies
      for(Map<String, Platform::Config::Target>::Node* i = config.targets.getFirst(); i; i = i->getNext())
      {
        Platform::Config::Target& target = i->data;
        for(const List<Platform::Config::Target::File>::Node* j = target.files.getFirst(); j; j = j->getNext())
          for(const List<String>::Node* i = j->data.dependencies.getFirst(); i; i = i->getNext())
            target.dependencies.append(i->data);
      }
    

    }
  return true;
}

bool Make::generateMetaMakefile()
{
  fileOpen("Makefile");
  fileWrite("\n");

  if(platforms.getSize() == 1)
    generateMetaMakefilePlatform(platforms.getFirst()->data);
  else if(platforms.getSize() > 1)
  {
    for(List<Platform>::Node* j = platforms.getLast(); j; j = j->getPrevious())
    {
      Platform& platform = j->data;
      if(j == platforms.getLast())
        fileWrite(String("ifeq ($(platform),") + platform.name + ")\n");
      else if(j == platforms.getFirst())
        fileWrite(String("else\n"));
      else 
        fileWrite(String("else ifeq ($(platform),") + platform.name + ")\n");
      fileWrite("\n");
      generateMetaMakefilePlatform(platform);
    }
    fileWrite("endif\n");
  }

  fileWrite("\n");
  fileClose();
  return true;
}

void Make::generateMetaMakefilePlatform(Platform& platform)
{
  if(platform.configs.getSize() == 1)
    generateMetaMakefileConfig(platform, platform.configs.getFirst()->data);
  else if(platform.configs.getSize() > 1)
  {
    for(List<Platform::Config>::Node* j = platform.configs.getLast(); j; j = j->getPrevious())
    {
      Platform::Config& config = j->data;
      if(j == platform.configs.getLast())
        fileWrite(String("ifeq ($(config),") + config.name + ")\n");
      else if(j == platform.configs.getFirst())
        fileWrite(String("else\n"));
      else
        fileWrite(String("else ifeq ($(config),") + config.name + ")\n");
      fileWrite("\n");
      generateMetaMakefileConfig(platform, config);
    }
    fileWrite("endif\n");
    fileWrite("\n");
  }
}

void Make::generateMetaMakefileConfig(const Platform& platform, const Platform::Config& config)
{
  fileWrite(String("include Makefile-") + platform.name + "-" + config.name + "\n");
  fileWrite("\n");
}

bool Make::generateMakefile(const Platform& platform, const Platform::Config& config)
{
  fileOpen(String("Makefile-") + platform.name + "-" + config.name);
  fileWrite("\n");

  fileWrite(".SUFFIXES:\n"); // disable some built-in rules

  String phony;
  for(List<Platform>::Node* k = platforms.getFirst(); k; k = 0)
    for(List<Platform::Config>::Node* j = k->data.configs.getFirst(); j; j = 0)
      for(Map<String, Platform::Config::Target>::Node* i = j->data.targets.getFirst(); i; i = i->getNext())
      {
        if(!phony.isEmpty())
          phony.append(' ');
        phony.append(i->data.name);
      }
  fileWrite(String(".PHONY: ") + phony + "\n");
  fileWrite("\n");

  for(const Map<String, Platform::Config::Target>::Node* i = config.targets.getFirst(); i; i = i->getNext())
  {
    const Platform::Config::Target& target = i->data;

    fileWrite(target.name + ": " + join(target.dependencies) + "\n");
    if(!target.files.isEmpty() || !target.command.isEmpty() || !target.output.isEmpty() || !target.type.isEmpty())
      fileWrite(String("\t@$(MAKE) --no-print-directory -r -f ") + target.name + "-" + platform.name + "-" + config.name + ".make\n");
    fileWrite("\n");
  }

  fileClose();
  return true;
}

bool Make::generateTargetMakefile(const Platform& platform, const Platform::Config& config, const Platform::Config::Target& target)
{
  fileOpen(target.name + "-" + platform.name + "-" + config.name + ".make");
  fileWrite("\n");

  fileWrite(".SUFFIXES:\n"); // disable some built-in rules
  fileWrite(String(".PHONY: ") + target.name + "\n");
  fileWrite("\n");

  if(!target.objects.isEmpty())
  {
    fileWrite(String("__SOFLAGS := ") + join(target.soFlags) + "\n");
    fileWrite(String("__CPPFLAGS := ") + join(target.cppFlags) + "\n");
    fileWrite(String("__CFLAGS := ") + join(target.cFlags) + "\n");
    fileWrite(String("__DEFINES := ") + join(target.defines, "-D") + "\n");
    fileWrite(String("__INCLUDEPATHS := ") + join(target.includePaths, "-I") + "\n");

    fileWrite(String("__OBJECTS := ") + join(target.objects) + "\n");
    fileWrite(String("__LINKFLAGS := ") + join(target.linkFlags) + "\n");
    fileWrite(String("__LIBPATHS := ") + join(target.libPaths, "-L") + "\n");
    fileWrite(String("__LIBS := ") + join(target.libs, "-l") + "\n");
    
    fileWrite("\n");
    
    if(!target.cppCompiler.isEmpty() || !target.linker.isEmpty())
    {
      fileWrite("ifeq ($(origin CXX),default)\n");
      if(!target.cppCompiler.isEmpty())
        fileWrite(String("  CXX := ") + join(target.cppCompiler) + "\n");
      if(!target.linker.isEmpty())
        fileWrite(String("  __CXXLINKER := ") + join(target.linker) + "\n");
      else
        fileWrite("  __CXXLINKER := $(CXX)\n");
      fileWrite("else\n");
      fileWrite("  __CXXLINKER := $(CXX)\n");
      fileWrite("endif\n");
    }
    else
      fileWrite("__CXXLINKER := $(CXX)\n");
    fileWrite("\n");
    
    if(!target.cCompiler.isEmpty() || !target.linker.isEmpty())
    {
      fileWrite("ifeq ($(origin CC),default)\n");
      if(!target.cCompiler.isEmpty())
        fileWrite(String("  CC := ") + join(target.cCompiler) + "\n");
      if(!target.linker.isEmpty())
        fileWrite(String("  __CCLINKER := ") + join(target.linker) + "\n");
      else
        fileWrite("  __CCLINKER := $(CC)\n");
      fileWrite("else\n");
      fileWrite("  __CCLINKER := $(CC)\n");
      fileWrite("endif\n");
    }
    else
      fileWrite("__CCLINKER := $(CC)\n");
    fileWrite("\n");
    
    if(!target.linker.isEmpty())
    {
      fileWrite("ifeq ($(origin AR),default)\n");
      fileWrite(String("  __ARLINKER := ") + join(target.linker) + "\n");
      fileWrite("endif\n");
    }
    else
      fileWrite("__ARLINKER := $(AR)\n");
    fileWrite("\n");
  }

  // add "linking" recipe
  String reqDirs;
  for(const List<String>::Node* i = target.output.getFirst(); i; i = i->getNext())
  {
    String dir = File::getDirname(i->data);
    if(dir != ".")
    {
      if(!reqDirs.isEmpty())
        reqDirs.append(' ');
      reqDirs.append(dir);
    }
  }

  String outputFile = target.output.isEmpty() ? String() : target.output.getFirst()->data;
  if(outputFile.isEmpty())
    fileWrite(target.name + ": $(__OBJECTS) " + join(target.nonObjects) + ";\n");
  else if(target.name != outputFile)
  {
    fileWrite(target.name + ": " + join(target.nonObjects) + " " + outputFile + "\n");
    fileWrite("\n");
    fileWrite(outputFile + ": $(__OBJECTS) " + join(target.input) + " | " + reqDirs + "\n");
  }
  else
  {
    fileWrite(target.name + ": " + join(target.nonObjects) + " __" + target.name + "\n");
    fileWrite("\n");
    fileWrite(String("__") + target.name + ": $(__OBJECTS) " + join(target.input) + " | " + reqDirs + "\n");
  }
  
  if(!outputFile.isEmpty())
  {
    bool hasMessage = !target.message.isEmpty();
    if(hasMessage)
      fileWrite(joinCommands("\t@echo \"", "\"\n", target.message));
    if(!target.command.isEmpty())
      fileWrite(joinCommands(hasMessage ? String("\t@") : String("\t"), "\n", target.command));
    fileWrite("\n");

    for(const List<String>::Node* i = target.output.getFirst()->getNext(); i; i = i->getNext())
    {
      fileWrite(i->data + ": " + outputFile + "\n");
      fileWrite("\t@touch $@\n");
      fileWrite("\n");
    }
  }
  else
    fileWrite("\n");

  // add recipe for creating output directories
  for(const Map<String, void*>::Node* i = target.outputDirs.getFirst(); i; i = i->getNext())
  {
    fileWrite(i->key + ":\n");
    fileWrite("\t@mkdir -p $@\n");
    fileWrite("\n");
  }

  // add recipe for each input file
  for(const List<Platform::Config::Target::File>::Node* i = target.files.getFirst(); i; i = i->getNext())
  {
    const Platform::Config::Target::File& file = i->data;

    if(!file.output.isEmpty())
    {
      const String& outputFile = file.output.getFirst()->data;

      String reqDirs;
      for(const List<String>::Node* i = file.output.getFirst(); i; i = i->getNext())
      {
        String dir = File::getDirname(i->data);
        if(dir != ".")
        {
          if(!reqDirs.isEmpty())
            reqDirs.append(' ');
          reqDirs.append(dir);
        }
      }

      fileWrite(outputFile + ": " + join(file.input) + " | " + reqDirs + "\n");
      bool hasMessage = !file.message.isEmpty();
      if(hasMessage)
        fileWrite(joinCommands("\t@echo \"", "\"\n", file.message));
      if(!file.command.isEmpty())
        fileWrite(joinCommands(hasMessage ? String("\t@") : String("\t"), "\n", file.command));
      fileWrite("\n");

      for(const List<String>::Node* i = file.output.getFirst()->getNext(); i; i = i->getNext())
      {
        fileWrite(i->data + ": " + outputFile + "\n");
        fileWrite("\t@touch $@\n");
        fileWrite("\n");
      }
    }
  }

  //
  if(!target.objects.isEmpty())
  {
    fileWrite("-include $(__OBJECTS:%.o=%.d)\n");
    fileWrite("\n");

    fileWrite("%.h: ;\n");
    fileWrite("%.hh: ;\n");
    fileWrite("%.hxx: ;\n");
    fileWrite("%.hpp: ;\n");
    fileWrite("%.c: ;\n");
    fileWrite("%.cc: ;\n");
    fileWrite("%.cxx: ;\n");
    fileWrite("%.cpp: ;\n");
    fileWrite("%.d: ;\n");
  }

  fileClose();
  return true;
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

String Make::join(const List<String>& items, const String& prefix)
{
  String result;
  const List<String>::Node* i = items.getFirst();
  if(i)
  {
    result = prefix;
    result.append(i->data);
    for(i = i->getNext(); i; i = i->getNext())
    {
      result.append(' ');
      result.append(prefix);
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
