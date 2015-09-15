
#include <cstdio>
#include <ctype.h>

#include "Mare.h"

#include "Tools/Assert.h"
#include "Tools/Process.h"
#include "Tools/File.h"
#include "Tools/Directory.h"
#include "Tools/Error.h"
#include "Engine.h"

bool Mare::build(const Map<String, String>& userArgs)
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

  // build 
  if(!buildFile())
    return false;

  return true;
}

bool Mare::buildFile()
{
  // enter root key
  engine.enterRootKey();

  // read default or check input platform names
  VERIFY(engine.enterKey("platforms"));
  if(inputPlatforms.isEmpty())
  {
    String firstPlatform = engine.getFirstKey();
    if(!firstPlatform.isEmpty())
      inputPlatforms.append(firstPlatform);
    else
    {
      engine.error("cannot find any platforms");
      return false;
    }
  }
  else
    for(const List<String>::Node* i = inputPlatforms.getFirst(); i; i = i->getNext())
      if(!engine.hasKey(i->data))
      {
        engine.error(String().format(256, "cannot find platform \"%s\"", i->data.getData()));
        return false;
      }
  engine.leaveKey();

  // read default or check input configuration names
  VERIFY(engine.enterKey("configurations"));
  if(inputConfigs.isEmpty())
  {
    String firstConfiguration = engine.getFirstKey();
    if(!firstConfiguration.isEmpty())
      inputConfigs.append(firstConfiguration);
    else
    {
      engine.error("cannot find any configurations");
      return false;
    }
  }
  else
    for(const List<String>::Node* i = inputConfigs.getFirst(); i; i = i->getNext())
      if(!engine.hasKey(i->data))
      {
        engine.error(String().format(256, "cannot find configuration \"%s\"", i->data.getData()));
        return false;
      }
  engine.leaveKey();

  // read default or check input target names
  VERIFY(engine.enterKey("targets"));
  engine.getKeys(allTargets);
  if(inputTargets.isEmpty())
  {
    if(!allTargets.isEmpty())
      inputTargets.append(allTargets.getFirst()->data);
    else
    {
      engine.error("cannot find any targets");
      return false;
    }
  }
  else
    for(const List<String>::Node* i = inputTargets.getFirst(); i; i = i->getNext())
      if(!engine.hasKey(i->data))
      {
        engine.error(String().format(256, "cannot find target \"%s\"", i->data.getData()));
        return false;
      }
  engine.leaveKey();

  // leave root key
  engine.leaveKey(); 

  // build input targets (with dependencies) foreach input configuration
  for(const List<String>::Node* i = inputPlatforms.getFirst(); i; i = i->getNext())
  {
    const String& platform = i->data;
    for(const List<String>::Node* i = inputConfigs.getFirst(); i; i = i->getNext())
    {
      const String& configuration = i->data;
      if(!buildTargets(platform, configuration))
        return false;
    }
  }

  return true;
}

class Target;

class Rule
{
public:
  const Mare* builder;
  Target* target;

  String name; /**< The main input file or the name of the target */
  List<String> dependencies;
  List<String> inputs;
  List<String> outputs;
  List<String> command;
  List<String> message;
  
  unsigned int finishedRuleDependencies;
  Map<Rule*, String> ruleDependencies;
  Map<Rule*, String> rulePropagations;

  bool rebuild;

  const List<String>::Node* nextCommand;
  Process process;

  Rule() : finishedRuleDependencies(0), rebuild(false) {}

  bool startExecution(unsigned int& pid)
  {
    ASSERT(finishedRuleDependencies == ruleDependencies.getSize());

    if(builder->clean)
      goto clean;
    if(builder->rebuild)
    {
      if(builder->showDebug)
        printf("debug: Applying rule for \"%s\" since the rebuild flag is set\n", name.getData());
      goto build;
    }

    // determine whether to build this rule
    for(Map<Rule*, String>::Node* i = ruleDependencies.getFirst(); i; i = i->getNext())
      if(i->key->rebuild)
      {
        if(builder->showDebug)
          printf("debug: Applying rule for \"%s\" since the rule for the input file \"%s\" was applied as well\n", name.getData(), i->data.getData());
        goto build;
      }
    if(!outputs.isEmpty())
    {
      long long minWriteTime = 0;
      String minOutputFile;
      for(const List<String>::Node* i = outputs.getFirst(); i; i = i->getNext())
      {
        const String& file = i->data;
        long long writeTime;
        if(!File::getWriteTime(file, writeTime))
        {
          if(builder->showDebug)
          {
            if(!File::exists(file))
              printf("debug: Applying rule for \"%s\" since the output file \"%s\" does not exist\n", name.getData(), file.getData());
            else
              printf("debug: Applying rule for \"%s\" since the last modification time of output file \"%s\" cannot be read\n", name.getData(), file.getData());
          }
          goto build;
        }
        if(i == outputs.getFirst() || writeTime < minWriteTime)
        {
          minWriteTime = writeTime;
          minOutputFile = file;
        }
      }
      for(const List<String>::Node* i = inputs.getFirst(); i; i = i->getNext())
      {
        const String& file = i->data;
        long long writeTime;
        if(!File::getWriteTime(file, writeTime))
        {
          if(builder->showDebug)
          {
            if(!File::exists(file))
              printf("debug: Applying rule for \"%s\" since the input file \"%s\" does not exist\n", name.getData(), file.getData());
            else
              printf("debug: Applying rule for \"%s\" since the last modification time of input file \"%s\" cannot be read\n", name.getData(), file.getData());
          }
          goto build;
        }
        if(writeTime > minWriteTime) // Do not rebuild if both files have the same write time. This will prevent mare from (e.g.) relinking output files on systems (e.g. windows)
                                     // with a timestamp resolutions of a second when compiling and linking can be done in less than a second.
        {
          if(builder->showDebug)
            printf("debug: Applying rule for \"%s\" since the input file \"%s\" is newer than output file \"%s\"\n", name.getData(), file.getData(), minOutputFile.getData());
          goto build;
        }
      }
    }

    // no rebuilding
    pid = 0;
    return true;
  
clean:

    // delete output files and directories
    for(const List<String>::Node* i = outputs.getFirst(); i; i = i->getNext())
    {
      if(File::exists(i->data))
      {
        if(!File::unlink(i->data))
          builder->engine.error(Error::getString());
      }
      if(!builder->rebuild)
      {
        /*
        String dir = File::getDirname(i->data);
        if(!unlinkDirs.find(dir)
          unlinkDirs.append(dir, 0);
        */
        Directory::remove(File::getDirname(i->data));
      }
    }

    if(builder->rebuild)
      goto build;
    pid = 0;
    return true;

    //
  build:
    this->rebuild = true;

    if(outputs.isEmpty())
    {
      pid = 0;
      return true; // that was easy
    }

    if(!message.isEmpty())
    {
      for(const List<String>::Node* i = message.getFirst(); i; i = i->getNext())
        puts(i->data.getData());
      fflush(stdout);
    }

    // create output directories
    for(const List<String>::Node* i = outputs.getFirst(); i; i = i->getNext())
      Directory::create(File::getDirname(i->data));

    nextCommand = command.getFirst();
    return continueExecution(pid);
  }

  bool continueExecution(unsigned int& pid)
  {
    if(process.isRunning())
    {
      unsigned int exitCode = process.join();
      if(exitCode != 0)
      {
        pid = 0;
        return false;
      }
    }

    String singleCommand;
    while(nextCommand)
    {
      singleCommand = nextCommand->data;
      nextCommand = nextCommand->getNext();
      if(!singleCommand.isEmpty())
        break;
    }

    if(singleCommand.isEmpty())
    {
      pid = 0;
      return true;
    }

    if(message.isEmpty())
    {
      puts(singleCommand.getData());
      fflush(stdout);
    }

    if(builder->showDebug)
    {
      printf("debug: %s\n", singleCommand.getData());
      fflush(stdout);
    }

    pid = process.start(singleCommand);
    if(!pid)
    {
      builder->engine.error(Error::getString());
      return false;
    }
    return true;
  }
};

class Target
{
public:
  List<Rule> rules;
  bool active;
  Rule* rule; /**< The final rule for the target (mostly used for linking) */

  Target() : active(false) {}
};

class RuleSet
{
public:
  Map<String, Target> targets;
  List<Target*> activeTargets;

  unsigned int activeRules;
  unsigned int finishedRules;

  RuleSet() : activeRules(0), finishedRules(0) {}

  void resolveDependencies(bool activateDependencies)
  {
    // generate outputToRule map
    Map<String, Rule*> outputToRule;
    for(Map<String, Target>::Node* i = targets.getFirst(); i; i = i->getNext())
      for(List<Rule>::Node* j = i->data.rules.getFirst(); j; j = j->getNext())
      {
        Rule& rule = j->data;
        if(rule.command.isEmpty() && !rule.outputs.isEmpty())
        {
          printf("warning: Rule for \"%s\" does not define a command\n", rule.name.getData());
        }
        for(List<String>::Node* i = rule.outputs.getFirst(); i; i = i->getNext())
        {
          if(outputToRule.find(i->data))
          {
            printf("warning: There are multiple rules for the output file \"%s\"\n", i->data.getData());
            continue;
          }
          outputToRule.append(i->data, &rule);
        }
      }

    // map input files to rules
    for(List<Target*>::Node* i = activeTargets.getFirst(); i; i = i->getNext())
      for(List<Rule>::Node* j = i->data->rules.getFirst(); j; j = j->getNext())
      {
        Rule& rule = j->data;
        ++activeRules;

        // convert "dependencies" to additional input files
        for(List<String>::Node* i = rule.dependencies.getFirst(); i; i = i->getNext())
        {
          Map<String, Target>::Node* node = targets.find(i->data);
          if(!node)
          {
            printf("warning: Cannot resolve dependency \"%s\" for the rule for \"%s\"\n", i->data.getData(), rule.name.getData());
            continue;
          }
          for(List<String>::Node* j = node->data.rule->outputs.getFirst(); j; j = j->getNext())
            rule.inputs.append(j->data);

          // activate dependency
          if(activateDependencies && !node->data.active)
          {
            node->data.active = true;
            activeTargets.append(&node->data);
          }
        }

        //
        for(List<String>::Node* i = rule.inputs.getFirst(); i; i = i->getNext())
        {
          Rule* dependency = outputToRule.lookup(i->data);
          if(dependency)
          {
            if(dependency == &rule)
            {
              //printf("warning: Rule for \"%s\" depends on itself\n", rule.name.getData());
              continue;
            }

            if(!dependency->target->active && !activateDependencies)
              continue;

            // activate target?
            if(!dependency->target->active)
            {
              dependency->target->active = true;
              activeTargets.append(dependency->target);
            }

            //
            if(!rule.ruleDependencies.find(dependency))
              rule.ruleDependencies.append(dependency, i->data);
            if(!dependency->rulePropagations.find(&rule))
              dependency->rulePropagations.append(&rule, i->data);
          }
        }
      }
  }
  
  bool build(Engine& engine, unsigned int maxParallelJobs, bool clean, bool rebuild, bool showDebug)
  {
    List<Rule*> pendingJobs;
    for(List<Target*>::Node* i = activeTargets.getFirst(); i; i = i->getNext())
      for(List<Rule>::Node* j = i->data->rules.getFirst(); j; j = j->getNext())
        if(j->data.ruleDependencies.isEmpty())
          pendingJobs.append(&j->data);
    
    Map<unsigned int, Rule*> runningJobs;
    bool failure = false;
    do
    {
      Rule* rule;

      if(!failure)
        while(runningJobs.getSize() < maxParallelJobs && !pendingJobs.isEmpty())
        {
          rule = pendingJobs.getFirst()->data;
          pendingJobs.removeFirst();
          unsigned int pid;
          if(!rule->startExecution(pid))
          {
            failure = true;
            goto finishedRuleExecution;
          }
          if(pid)
            runningJobs.append(pid, rule);
          else
            goto finishedRuleExecution;
        }

      if(!runningJobs.isEmpty())
      {
        unsigned int pid = Process::waitOne();
        Map<unsigned int, Rule*>::Node* job = runningJobs.find(pid);
        if(!job)
          continue;
        rule = job->data;
        runningJobs.remove(job);
        if(!rule->continueExecution(pid))
        {
          failure = true;
          goto finishedRuleExecution;
        }
        if(pid)
          runningJobs.append(pid, rule);
        else
          goto finishedRuleExecution;
      }
      continue;

    finishedRuleExecution:
      ++finishedRules;
      for(Map<Rule*, String>::Node* i = rule->rulePropagations.getFirst(); i; i = i->getNext())
      {
        Rule& rule = *i->key;
        ASSERT(!rule.ruleDependencies.isEmpty());
        ++rule.finishedRuleDependencies;
        if(rule.finishedRuleDependencies == rule.ruleDependencies.getSize())
        {
          if(rule.rulePropagations.isEmpty())
            pendingJobs.append(&rule);
          else
            pendingJobs.prepend(&rule);
        }
      }
    } while(!runningJobs.isEmpty() || (!pendingJobs.isEmpty() && !failure));

    if(failure)
      return false;

    // unresolvable dependencies?
    if(finishedRules < activeRules)
    {
      engine.error("cannot build some targets because of circular dependencies");
      return false;
    }

    return true;;
  }
};

bool Mare::buildTargets(const String& platform, const String& configuration)
{
  RuleSet ruleSet;

  Map<String, void*> activateTargets;
  for(const List<String>::Node* i = inputTargets.getFirst(); i; i = i->getNext())
    activateTargets.append(i->data, 0);

  List<String> files;
  for(List<String>::Node* i = allTargets.getFirst(); i; i = i->getNext())
  {
    engine.enterUnnamedKey();
    engine.addDefaultKey("platform", platform);
    engine.addDefaultKey(platform, platform);
    engine.addDefaultKey("configuration", configuration);
    engine.addDefaultKey(configuration, configuration);
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

    Target& target = ruleSet.targets.append(i->data);
    if(activateTargets.find(i->data))
    {
      target.active = true;
      ruleSet.activeTargets.append(&target);
    }
    
    // add rule for each source file
    if(engine.enterKey("files"))
    {
      files.clear();
      engine.getKeys(files);
      for(List<String>::Node* i = files.getFirst(); i; i = i->getNext())
      {
        Rule& rule = target.rules.append();
        rule.builder = this;
        rule.target = &target;
        rule.name = i->data;
        engine.enterUnnamedKey();
        engine.addDefaultKey("file", i->data);
        VERIFY(engine.enterKey(i->data));
        engine.getKeys("dependencies", rule.dependencies, false);
        engine.getKeys("input", rule.inputs, false);
        engine.getKeys("output", rule.outputs, false);
        engine.getText("command", rule.command, false);
        engine.getText("message", rule.message, false);
        engine.leaveKey(); // VERIFY(engine.enterKey(i->data));
        engine.leaveKey();
      }
      engine.leaveKey();
    }

    // add rule for target file
    Rule& rule = target.rules.append();
    rule.builder = this;
    rule.target = &target;
    rule.name = i->data;
    target.rule = &rule;
    engine.getKeys("dependencies", rule.dependencies, false);
    engine.getKeys("input", rule.inputs, false);
    engine.getKeys("output", rule.outputs, false);
    engine.getText("command", rule.command, false);
    engine.getText("message", rule.message, false);

    engine.leaveKey();
    engine.leaveKey();
    engine.leaveKey();
    engine.leaveKey();
  }

  ruleSet.resolveDependencies(!ignoreDependencies);
  return ruleSet.build(engine, jobs <= 0 ? (Process::getProcessorCount() - jobs) : jobs, clean, rebuild, showDebug);
}

String Mare::join(const List<String>& words)
{
  size_t totalLen = words.getSize() * 3;
  for(const List<String>::Node* i = words.getFirst(); i; i = i->getNext())
    totalLen += i->data.getLength();

  String result(totalLen);
  for(const List<String>::Node* i = words.getFirst(); i; i = i->getNext())
  {
    if(!result.isEmpty())
      result.append(' ');
    size_t len = result.getLength();
    char* dest = result.getData(len) + len; 
    for(const char* str = i->data.getData(); *str; ++str)
      if(isspace(*(unsigned char*)str))
      {
        result.setLength(len); // fall back
        result.append('"');
        result.append(i->data);
        result.append('"');
        goto next;
      }
      else
        *(dest++) = *str;
    result.setLength(len + i->data.getLength());
  next:;
  }
  return result;
}
