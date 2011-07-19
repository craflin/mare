
#include <cstdio>
#include <cassert>

#include "Builder.h"

#include "Tools/Process.h"
#include "Tools/File.h"
#include "Tools/Directory.h"
#include "Tools/Words.h"
#include "Engine.h"

bool Builder::build(const Map<String, String>& userArgs)
{
  // add default rules and stuff
  engine.addDefaultKey("CC", "gcc");
  engine.addDefaultKey("CXX", "g++");
  engine.addDefaultKey("AR", "ar");
  engine.enterDefaultKey("configurations");
    engine.addResolvableKey("Debug");
    engine.addResolvableKey("Release");
  engine.leaveKey();
  engine.addDefaultKey("targets");
  engine.addDefaultKey("buildDir", "$(configuration)");
  engine.addDefaultKey("cppFlags", "-Wall $(if $(Debug),-g,-Os -fomit-frame-pointer)");
  engine.addDefaultKey("linkFlags", "$(if $(Debug),,-s)");
  engine.enterDefaultKey("cppApplication");
    engine.addResolvableKey("inputs", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    engine.addResolvableKey("outputs", "$(buildDir)/$(target)$(if $(Win32),.exe)");
    engine.addResolvableKey("command", "$(CXX) -o $(outputs) $(inputs) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    engine.addResolvableKey("message", "Linking $(target)...");
  engine.leaveKey();
  engine.enterDefaultKey("cppDynamicLibrary");
    engine.addResolvableKey("inputs", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    engine.addResolvableKey("outputs", "$(buildDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.dll,.so)");
    engine.addResolvableKey("__soFlags", "$(if $(Win32),,-fpic)");
    engine.addResolvableKey("command", "$(CXX) -shared $(__soFlags) -o $(outputs) $(inputs) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    engine.addResolvableKey("message", "Linking $(target)...");
  engine.leaveKey();
  engine.enterDefaultKey("cppStaticLibrary");
    engine.addResolvableKey("inputs", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    engine.addResolvableKey("outputs", "$(buildDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.lib,.a)");
    engine.addResolvableKey("command", "$(AR) rcs $(outputs) $(inputs)");
    engine.addResolvableKey("message", "Creating $(target)...");
  engine.leaveKey();
  engine.enterDefaultKey("cApplication");
    engine.addResolvableKey("inputs", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    engine.addResolvableKey("outputs", "$(buildDir)/$(target)$(if $(Win32),.exe)");
    engine.addResolvableKey("command", "$(CC) -o $(outputs) $(inputs) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    engine.addResolvableKey("message", "Linking $(target)...");
  engine.leaveKey();
  engine.enterDefaultKey("cDynamicLibrary");
    engine.addResolvableKey("inputs", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    engine.addResolvableKey("outputs", "$(buildDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.dll,.so)");
    engine.addResolvableKey("__soFlags", "$(if $(Win32),,-fpic)");
    engine.addResolvableKey("command", "$(CC) -shared $(__soFlags) -o $(outputs) $(inputs) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    engine.addResolvableKey("message", "Linking $(target)...");
  engine.leaveKey();
  engine.enterDefaultKey("cStaticLibrary");
    engine.addResolvableKey("inputs", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    engine.addResolvableKey("outputs", "$(buildDir)/$(if $(Win32),,lib)$(patsubst lib%,%,$(target))$(if $(Win32),.lib,.a)");
    engine.addResolvableKey("command", "$(AR) rcs $(outputs) $(inputs)");
    engine.addResolvableKey("message", "Creating $(target)...");
  engine.leaveKey();
  engine.enterDefaultKey("cppSource");
    engine.addResolvableKey("__ofile", "$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file)))");
    engine.addResolvableKey("__dfile", "$(patsubst %.o,%.d,$(__ofile))");
    engine.addResolvableKey("inputs", "$(file) $(filter-out %.o: \\,$(readfile $(__dfile)))");
    engine.addResolvableKey("outputs", "$(__ofile) $(__dfile)");
    engine.addResolvableKey("command", "$(CXX) -MMD $(__soFlags) -o $(__ofile) -c $(file) $(cppFlags) $(CXXFLAGS) $(patsubst %,-D%,$(defines)) $(patsubst %,-I%,$(includePaths))");
    engine.addResolvableKey("message", "$(file)");
  engine.leaveKey();
  engine.enterDefaultKey("cSource");
    engine.addResolvableKey("__ofile", "$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file)))");
    engine.addResolvableKey("__dfile", "$(patsubst %.o,%.d,$(__ofile))");
    engine.addResolvableKey("inputs", "$(file) $(filter-out %.o: \\,$(readfile $(__dfile)))");
    engine.addResolvableKey("outputs", "$(__ofile) $(__dfile)");
    engine.addResolvableKey("command", "$(CC) -MMD $(__soFlags) -o $(__ofile) -c $(file) $(cFlags) $(CFLAGS) $(patsubst %,-D%,$(defines)) $(patsubst %,-I%,$(includePaths))");
    engine.addResolvableKey("message", "$(file)");
  engine.leaveKey();
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
  engine.addDefaultKey("platform", platform); // the target platform of the compiler

  // add user arguments
  engine.enterUnnamedKey();
  for(const Map<String, String>::Node* i = userArgs.getFirst(); i; i = i->getNext())
    engine.addDefaultKey(i->key, i->data);

  // get targets and configurations to build
  engine.getKeys("platform", inputPlatforms);
  engine.getKeys("configuration", inputConfigs);
  engine.getKeys("target", inputTargets);

  // build 
  if(!buildFile())
    return false;

  return true;
}

bool Builder::buildFile()
{
  for(const List<String>::Node* i = inputPlatforms.getFirst(); i; i = i->getNext())
  {
    const String& platform = i->data;
    engine.enterUnnamedKey();
    engine.addDefaultKey("platform", platform);
    engine.addDefaultKey(platform, "true"); // temp

    if(!engine.enterKey("configurations"))
    {
      assert(false);
      return false;
    }

    if(inputConfigs.isEmpty())
    {
      String firstConfiguration = engine.getFirstKey();
      if(!firstConfiguration.isEmpty())
      {
        engine.enterKey(firstConfiguration);
        bool result = buildConfiguration(firstConfiguration);
        engine.leaveKey();
        return result;
      }
      else
      {
        engine.error("cannot find any configurations");
        return false;
      }
    }

    if(!buildConfigurations())
      return false;

    engine.leaveKey();
    engine.leaveKey();
  }

  return true;
}

bool Builder::buildConfigurations()
{
  for(const List<String>::Node* i = inputConfigs.getFirst(); i; i = i->getNext())
  {
    if(!engine.enterKey(i->data))
    {
      String message;
      message.format(256, "cannot find configuration \"%s\"", i->data.getData());
      engine.error(message);
      return false;
    }
    if(!buildConfiguration(i->data))
      return false;
    engine.leaveKey();
  }
  return true;
}

bool Builder::buildConfiguration(const String& configuration)
{
  engine.addDefaultKey("configuration", configuration);
  engine.addDefaultKey(configuration, "true"); // temp

  if(!engine.enterKey("targets"))
  {
    assert(false);
    return false;
  }

  if(inputTargets.isEmpty())
  { // build all
    engine.getKeys(inputTargets);
    if(inputTargets.isEmpty())
    {
      engine.error("cannot find any targets");
      return false;
    }
    return buildTargets();
  }

  for(const List<String>::Node* node = inputTargets.getFirst(); node; node = node->getNext())
  {
    if(!engine.enterKey(node->data))
    {
      String message;
      message.format(256, "cannot find target \"%s\"", node->data.getData());
      engine.error(message);
      return false;
    }
    engine.leaveKey();
  }
  return buildTargets();
}

class Target;

class Rule
{
public:
  Target* target;

  String name; /**< The main input file or the name of the target */
  List<String> targetdeps;
  List<String> inputs;
  List<String> outputs;
  List<String> command;
  List<String> message;

  unsigned int finishedDependencies;
  Map<Rule*, String> dependencies;
  Map<Rule*, String> propagations;

  bool rebuild;

  Process process;

  Rule() : finishedDependencies(0), rebuild(false) {}

  bool startExecution(Engine& engine, unsigned int& pid, bool clean, bool rebuild, bool showDebug)
  {
    assert(finishedDependencies == dependencies.getSize());

    if(clean)
      goto clean;
    if(rebuild)
    {
      if(showDebug)
        printf("debug: Applying rule for \"%s\" since the rebuild flag is set\n", name.getData());
      goto build;
    }

    // determine whether to build this rule
    for(Map<Rule*, String>::Node* i = dependencies.getFirst(); i; i = i->getNext())
      if(i->key->rebuild)
      {
        if(showDebug)
          printf("debug: Applying rule for \"%s\" since the rule for the input file \"%s\" was applied as well\n", name.getData(), i->data.getData());
        goto build;
      }
    {
      long long minWriteTime = 0;
      String minOutputFile;
      for(const List<String>::Node* i = outputs.getFirst(); i; i = i->getNext())
      {
        const String& file = i->data;
        long long writeTime;
        if(!File::getWriteTime(file, writeTime))
        {
          if(showDebug)
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
          if(showDebug)
          {
            if(!File::exists(file))
              printf("debug: Applying rule for \"%s\" since the input file \"%s\" does not exist\n", name.getData(), file.getData());
            else
              printf("debug: Applying rule for \"%s\" since the last modification time of input file \"%s\" cannot be read\n", name.getData(), file.getData());
          }
          goto build;
        }
        if(writeTime >= minWriteTime)
        {
          if(showDebug)
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
        File file;
        if(!file.unlink(i->data))
        {
          engine.error(file.getErrno().getString());
        }
      }
      if(!rebuild)
      {
        /*
        String dir = File::getDirname(i->data);
        if(!unlinkDirs.find(dir)
          unlinkDirs.append(dir, 0);
        */
        Directory::remove(File::getDirname(i->data));
      }
    }

    if(rebuild)
      goto build;
    pid = 0;
    return true;

    //
  build:
    this->rebuild = true;

    String message;
    Words::append(this->message.isEmpty() ? this->command : this->message, message);
    puts(message.getData());

    if(showDebug)
    {
      String command;
      Words::append(this->command, command);
      printf("debug: %s\n", command.getData());
    }

    if(command.isEmpty())
    {
      pid = 0;
      return true;
    }

    // create output directories
    for(const List<String>::Node* i = outputs.getFirst(); i; i = i->getNext())
      Directory::create(File::getDirname(i->data));

    pid = process.start(command);
    if(!pid)
    {
      engine.error(process.getErrno().getString());
      return false;
    }
    return true;
  }

  bool finishExecution()
  {
    unsigned int exitCode = process.join();
    return exitCode == 0;
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

  void resolveDependencies()
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
        for(List<String>::Node* i = rule.targetdeps.getFirst(); i; i = i->getNext())
        {
          Map<String, Target>::Node* node = targets.find(i->data);
          if(!node)
          {
            printf("warning: Cannot resolve dependency \"%s\" for the rule for \"%s\"\n", i->data.getData(), rule.name.getData());
            continue;
          }
          for(List<String>::Node* j = node->data.rule->outputs.getFirst(); j; j = j->getNext())
            rule.inputs.append(j->data);
        }
      }

    // map input files to rules
    for(List<Target*>::Node* i = activeTargets.getFirst(); i; i = i->getNext())
      for(List<Rule>::Node* j = i->data->rules.getFirst(); j; j = j->getNext())
      {
        Rule& rule = j->data;
        ++activeRules;
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

            // activate target?
            if(!dependency->target->active)
            {
              dependency->target->active = true;
              activeTargets.append(dependency->target);
            }

            //
            if(!rule.dependencies.find(dependency))
              rule.dependencies.append(dependency, i->data);
            if(!dependency->propagations.find(&rule))
              dependency->propagations.append(&rule, i->data);
          }
        }
      }
  }
  
  bool build(Engine& engine, unsigned int maxParallelJobs, bool clean, bool rebuild, bool showDebug)
  {
    List<Rule*> pendingJobs;
    for(List<Target*>::Node* i = activeTargets.getFirst(); i; i = i->getNext())
      for(List<Rule>::Node* j = i->data->rules.getFirst(); j; j = j->getNext())
        if(j->data.dependencies.isEmpty())
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
          if(!rule->startExecution(engine, pid, clean, rebuild, showDebug))
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
        if(!rule->finishExecution())
          failure = true;
        goto finishedRuleExecution;
      }
      continue;

    finishedRuleExecution:
      ++finishedRules;
      for(Map<Rule*, String>::Node* i = rule->propagations.getFirst(); i; i = i->getNext())
      {
        Rule& rule = *i->key;
        assert(!rule.dependencies.isEmpty());
        ++rule.finishedDependencies;
        if(rule.finishedDependencies == rule.dependencies.getSize())
        {
          if(rule.propagations.isEmpty())
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

bool Builder::buildTargets()
{
  RuleSet ruleSet;

  Map<String, void*> activateTargets;
  for(const List<String>::Node* i = inputTargets.getFirst(); i; i = i->getNext())
    activateTargets.append(i->data, 0);

  List<String> targets;
  engine.getKeys(targets);
  List<String> files;
  for(List<String>::Node* i = targets.getFirst(); i; i = i->getNext())
  {
    Target& target = ruleSet.targets.append(i->data);
    if(activateTargets.find(i->data))
    {
      target.active = true;
      ruleSet.activeTargets.append(&target);
    }
    engine.enterKey(i->data);
    engine.addDefaultKey("target", i->data);

    // add rule for each source file
    if(engine.enterKey("files"))
    {
      files.clear();
      engine.getKeys(files);
      for(List<String>::Node* i = files.getFirst(); i; i = i->getNext())
      {
        Rule& rule = target.rules.append();
        rule.target = &target;
        rule.name = i->data;
        engine.enterKey(i->data);
        engine.addDefaultKey("file", i->data);
        engine.getKeys("dependencies", rule.targetdeps, false);
        engine.getKeys("inputs", rule.inputs, false);
        engine.getKeys("outputs", rule.outputs, false);
        engine.getKeys("command", rule.command, false);
        engine.getKeys("message", rule.message, false);
        engine.leaveKey();
      }
      engine.leaveKey();
    }

    // add rule for target file
    Rule& rule = target.rules.append();
    rule.target = &target;
    rule.name = i->data;
    target.rule = &rule;
    engine.getKeys("dependencies", rule.targetdeps, false);
    engine.getKeys("inputs", rule.inputs, false);
    engine.getKeys("outputs", rule.outputs, false);
    engine.getKeys("command", rule.command, false);
    engine.getKeys("message", rule.message, false);

    engine.leaveKey();
  }

  ruleSet.resolveDependencies();
  return ruleSet.build(engine, jobs <= 0 ? (Process::getProcessorCount() - jobs) : jobs, clean, rebuild, showDebug);
}

