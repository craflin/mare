
#include "Tools/Assert.h"
#include "Tools/Directory.h"
#include "Tools/File.h"
#include "Tools/Process.h"
#include "Engine.h"

#include "JsonDb.h"

bool JsonDb::generate(const Map<String, String>& userArgs)
{
  // add default rules and stuff
  engine.addDefaultKey("cCompiler", "gcc");
  engine.addDefaultKey("cppCompiler", "g++");
  engine.addDefaultKey("configurations", "Debug Release");
  engine.addDefaultKey("targets"); // an empty target list exists per default
  engine.addDefaultKey("buildDir", "$(configuration)");
  engine.addDefaultKey("outputDir", "$(buildDir)");
  engine.addDefaultKey("cFlags", "-Wall $(if $(Debug),-g,-Os -fomit-frame-pointer)");
  engine.addDefaultKey("cppFlags", "-Wall $(if $(Debug),-g,-Os -fomit-frame-pointer)");
  engine.addDefaultKey("linkFlags", "$(if $(Debug),,-s)");
  {
    Map<String, String> cppApplication;
    cppApplication.append("input", "$(addprefix $(buildDir)/,$(addsuffix .o,$(basename $(subst ../,,$(filter %.c%,$(files))))))");
    cppApplication.append("output", "$(outputDir)/$(target)$(if $(Win32),.exe)");
    cppApplication.append("command", "$(linker) -o $(output) $(input) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    cppApplication.append("message", "-> $(output)");
    cppApplication.append("linker", "$(if $(linker),$(linker),$(cppCompiler))");
    engine.addDefaultKey("cppApplication", cppApplication);
  }
  {
    Map<String, String> cppDynamicLibrary;
    cppDynamicLibrary.append("input", "$(addprefix $(buildDir)/,$(addsuffix .o,$(basename $(subst ../,,$(filter %.c%,$(files))))))");
    cppDynamicLibrary.append("output", "$(outputDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.dll,.so)");
    cppDynamicLibrary.append("__soFlags", "$(if $(Win32),,-fpic)");
    cppDynamicLibrary.append("command", "$(linker) -shared $(__soFlags) -o $(output) $(input) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    cppDynamicLibrary.append("message", "-> $(output)");
    cppDynamicLibrary.append("linker", "$(if $(linker),$(linker),$(cppCompiler))");
    engine.addDefaultKey("cppDynamicLibrary", cppDynamicLibrary);
  }
  {
    Map<String, String> cppStaticLibrary;
    cppStaticLibrary.append("input", "$(addprefix $(buildDir)/,$(addsuffix .o,$(basename $(subst ../,,$(filter %.c%,$(files))))))");
    cppStaticLibrary.append("output", "$(outputDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.lib,.a)");
    cppStaticLibrary.append("command", "$(linker) rcs $(output) $(input)");
    cppStaticLibrary.append("message", "-> $(output)");
    cppStaticLibrary.append("linker", "$(if $(linker),$(linker),ar)");
    engine.addDefaultKey("cppStaticLibrary", cppStaticLibrary);
  }
  {
    Map<String, String> cApplication;
    cApplication.append("input", "$(addprefix $(buildDir)/,$(addsuffix .o,$(basename $(subst ../,,$(filter %.c%,$(files))))))");
    cApplication.append("output", "$(outputDir)/$(target)$(if $(Win32),.exe)");
    cApplication.append("command", "$(linker) -o $(output) $(input) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    cApplication.append("message", "-> $(output)");
    cApplication.append("linker", "$(if $(linker),$(linker),$(cCompiler))");
    engine.addDefaultKey("cApplication", cApplication);
  }
  {
    Map<String, String> cDynamicLibrary;
    cDynamicLibrary.append("input", "$(addprefix $(buildDir)/,$(addsuffix .o,$(basename $(subst ../,,$(filter %.c%,$(files))))))");
    cDynamicLibrary.append("output", "$(outputDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.dll,.so)");
    cDynamicLibrary.append("__soFlags", "$(if $(Win32),,-fpic)");
    cDynamicLibrary.append("command", "$(linker) -shared $(__soFlags) -o $(output) $(input) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    cDynamicLibrary.append("message", "-> $(output)");
    cDynamicLibrary.append("linker", "$(if $(linker),$(linker),$(cCompiler))");
    engine.addDefaultKey("cDynamicLibrary", cDynamicLibrary);
  }
  {
    Map<String, String> cStaticLibrary;
    cStaticLibrary.append("input", "$(addprefix $(buildDir)/,$(addsuffix .o,$(basename $(subst ../,,$(filter %.c%,$(files))))))");
    cStaticLibrary.append("output", "$(outputDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.lib,.a)");
    cStaticLibrary.append("command", "$(linker) rcs $(output) $(input)");
    cStaticLibrary.append("message", "-> $(output)");
    cStaticLibrary.append("linker", "$(if $(linker),$(linker),ar)");
    engine.addDefaultKey("cStaticLibrary", cStaticLibrary);
  }
  {
    Map<String, String> cppSource;
    cppSource.append("__ofile", "$(buildDir)/$(basename $(subst ../,,$(file))).o");
    cppSource.append("__dfile", "$(patsubst %.o,%.d,$(__ofile))");
    cppSource.append("input", "$(file) $(filter-out %.o: \\,$(readfile $(__dfile)))");
    cppSource.append("output", "$(__ofile) $(__dfile)");
    cppSource.append("command", "$(cppCompiler) -MMD $(__soFlags) -o $(__ofile) -c $(file) $(cppFlags) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(patsubst %,-D%,$(defines)) $(patsubst %,-I%,$(includePaths))");
    cppSource.append("message", "$(subst ./,,$(file))");
    engine.addDefaultKey("cppSource", cppSource);
  }
  {
    Map<String, String> cSource;
    cSource.append("__ofile", "$(buildDir)/$(basename $(subst ../,,$(file))).o");
    cSource.append("__dfile", "$(patsubst %.o,%.d,$(__ofile))");
    cSource.append("input", "$(file) $(filter-out %.o: \\,$(readfile $(__dfile)))");
    cSource.append("output", "$(__ofile) $(__dfile)");
    cSource.append("command", "$(cCompiler) -MMD $(__soFlags) -o $(__ofile) -c $(file) $(cFlags) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(patsubst %,-D%,$(defines)) $(patsubst %,-I%,$(includePaths))");
    cSource.append("message", "$(subst ./,,$(file))");
    engine.addDefaultKey("cSource", cSource);
  }
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

  String architecture = Process::getArchitecture();
  engine.addDefaultKey("architecture", architecture);
  engine.addDefaultKey("arch", architecture);

  // add user arguments
  for(const Map<String, String>::Node* i = userArgs.getFirst(); i; i = i->getNext())
    engine.addCommandLineKey(i->key, i->data);

  engine.enterRootKey();
  List<String> allPlatforms, allConfigurations, allTargets;
  engine.getKeys("platforms", allPlatforms);
  engine.getKeys("configurations", allConfigurations);
  engine.getKeys("targets", allTargets);
  engine.leaveKey();

  List<String> files, inputs, outputs, command;

  File jsonCompilationDatabase;

  for(const List<String>::Node* i = allPlatforms.getFirst(); i; i = i->getNext())
  {
    const String& platform = i->data;
    for(const List<String>::Node* i = allConfigurations.getFirst(); i; i = i->getNext())
    {
      const String& configuration = i->data;
      for(const List<String>::Node* i = allTargets.getFirst(); i; i = i->getNext())
      {
        const String& target = i->data;

        engine.enterUnnamedKey();
        engine.addDefaultKey("platform", platform);
        engine.addDefaultKey(platform, platform);
        engine.addDefaultKey("configuration", configuration);
        engine.addDefaultKey(configuration, configuration);
        engine.addDefaultKey("target", target);

        engine.enterRootKey();
        VERIFY(engine.enterKey("targets"));
        if(!engine.enterKey(target))
        {
          engine.error(String().format(256, "cannot find target \"%s\"", i->data.getData()));
          return false;
        }

        String mareDir = engine.getMareDir();
        engine.addDefaultKey("mareDir", mareDir);

        if(engine.enterKey("files"))
        {
          String outputDir = engine.getFirstKey("outputDir", true);
          if(!Directory::exists(outputDir) && !Directory::create(outputDir))
            return false;

          VERIFY(jsonCompilationDatabase.open(outputDir + "/compile_commands.json", File::writeFlag));
          jsonCompilationDatabase.write("[", 1);

          bool first = true;

          files.clear();
          engine.getKeys(files);
          for(List<String>::Node* i = files.getFirst(); i; i = i->getNext())
          {
            inputs.clear();
            outputs.clear();
            command.clear();
            engine.enterUnnamedKey();
            engine.addDefaultKey("file", i->data);
            VERIFY(engine.enterKey(i->data));
            engine.getKeys("input", inputs, false);
            engine.getKeys("output", outputs, false);
            engine.getText("command", command, false);

            if(!inputs.isEmpty() && !outputs.isEmpty() && command.getSize() == 1)
            {
              if(!first)
                jsonCompilationDatabase.write(",", 1);
              else
                first = false;
              jsonCompilationDatabase.write("\n{\n  \"directory\": \"", 19);
              writeJsonEscaped(jsonCompilationDatabase, Directory::getCurrent() + "/" + mareDir);
              jsonCompilationDatabase.write("\",\n  \"command\": \"", 17);
              writeJsonEscaped(jsonCompilationDatabase, command.getFirst()->data);
              jsonCompilationDatabase.write("\",\n  \"file\": \"", 14);
              writeJsonEscaped(jsonCompilationDatabase, inputs.getFirst()->data);
              jsonCompilationDatabase.write("\"\n}", 3);
            }

            engine.leaveKey();
            engine.leaveKey();
          }
          engine.leaveKey();

          jsonCompilationDatabase.write("\n]\n", 3);
          jsonCompilationDatabase.close();
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

void JsonDb::writeJsonEscaped(File& f, const String& string)
{
  const char* data = string.getData();
  for(size_t i = 0; i < string.getLength(); i++)
  {
    if(data[i] == '\\')
      f.write("\\\\", 2);
    else if(data[i] == '\"')
      f.write("\\\"", 2);
    else
      f.write(data + i, 1);
  }
}
