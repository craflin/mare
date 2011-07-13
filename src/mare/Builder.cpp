
#include <cstdio>
#include <cassert>

#include "Builder.h"

#include "../libmare/Tools/Process.h"
#include "../libmare/Tools/File.h"
#include "../libmare/Tools/Directory.h"
#include "../libmare/Tools/Words.h"
#include "../libmare/Engine.h"

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
  engine.enterDefaultKey("cppApplication");
    engine.addResolvableKey("input", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    engine.addResolvableKey("output", "$(buildDir)/$(target)$(if $(windows),.exe)");
    engine.addResolvableKey("command", "$(CXX) -o $(output) $(input) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    engine.addResolvableKey("message", "Linking $(target)...");
  engine.leaveKey();
  engine.enterDefaultKey("cppDynamicLibrary");
    engine.addResolvableKey("input", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    engine.addResolvableKey("output", "$(buildDir)/$(if $(windows),,lib)$(target)$(if $(windows),.dll,.so)");
    engine.addResolvableKey("__soFlags", "-fpic");
    engine.addResolvableKey("command", "$(CXX) -shared $(__soFlags) -o $(output) $(input) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    engine.addResolvableKey("message", "Linking $(target)...");
  engine.leaveKey();
  engine.enterDefaultKey("cppStaticLibrary");
    engine.addResolvableKey("input", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    engine.addResolvableKey("output", "$(buildDir)/$(if $(windows),,lib)$(target)$(if $(windows),.lib,.a)");
    engine.addResolvableKey("command", "$(AR) rcs $(output) $(input)");
    engine.addResolvableKey("message", "Creating $(target)...");
  engine.leaveKey();
  engine.enterDefaultKey("cApplication");
    engine.addResolvableKey("input", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    engine.addResolvableKey("output", "$(buildDir)/$(target)$(if $(windows),.exe)");
    engine.addResolvableKey("command", "$(CC) -o $(output) $(input) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    engine.addResolvableKey("message", "Linking $(target)...");
  engine.leaveKey();
  engine.enterDefaultKey("cDynamicLibrary");
    engine.addResolvableKey("input", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    engine.addResolvableKey("output", "$(buildDir)/$(if $(windows),,lib)$(target)$(if $(windows),.dll,.so)");
    engine.addResolvableKey("__soFlags", "-fpic");
    engine.addResolvableKey("command", "$(CC) -shared $(__soFlags) -o $(output) $(input) $(linkFlags) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
    engine.addResolvableKey("message", "Linking $(target)...");
  engine.leaveKey();
  engine.enterDefaultKey("cStaticLibrary");
    engine.addResolvableKey("input", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
    engine.addResolvableKey("output", "$(buildDir)/$(if $(windows),,lib)$(target)$(if $(windows),.lib,.a)");
    engine.addResolvableKey("command", "$(AR) rcs $(output) $(input)");
    engine.addResolvableKey("message", "Creating $(target)...");
  engine.leaveKey();
  engine.enterDefaultKey("cppSource");
    engine.addResolvableKey("__ofile", "$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file)))");
    engine.addResolvableKey("__dfile", "$(patsubst %.o,%.d,$(__ofile))");
    engine.addResolvableKey("input", "$(file) $(filter-out %.o: \\,$(readfile $(__dfile)))");
    engine.addResolvableKey("output", "$(__ofile) $(__dfile)");
    engine.addResolvableKey("command", "$(CXX) -MMD $(__soFlags) -o $(__ofile) -c $(file) $(cppFlags) $(CXXFLAGS) $(patsubst %,-D%,$(defines)) $(patsubst %,-I%,$(includePaths))");
    engine.addResolvableKey("message", "$(file)");
  engine.leaveKey();
  engine.enterDefaultKey("cSource");
    engine.addResolvableKey("__ofile", "$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file)))");
    engine.addResolvableKey("__dfile", "$(patsubst %.o,%.d,$(__ofile))");
    engine.addResolvableKey("input", "$(file) $(filter-out %.o: \\,$(readfile $(__dfile)))");
    engine.addResolvableKey("output", "$(__ofile) $(__dfile)");
    engine.addResolvableKey("command", "$(CC) -MMD $(__soFlags) -o $(__ofile) -c $(file) $(cFlags) $(CFLAGS) $(patsubst %,-D%,$(defines)) $(patsubst %,-I%,$(includePaths))");
    engine.addResolvableKey("message", "$(file)");
  engine.leaveKey();
#if defined(_WIN32)
  String platform("win32");
#elif defined(__linux)
  String platform("linux");
#elif defined(__APPLE__) && defined(__MACH__)
  String platform("macosx");
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
    engine.resetKey();
    engine.addDefaultKey("platform", platform);
    engine.addDefaultKey(platform, "true");

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
    {
      engine.leaveKey();
      return false;
    }
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
    {
      engine.leaveKey();
      return false;
    }
    engine.leaveKey();
  }
  return true;
}

bool Builder::buildConfiguration(const String& configuration)
{
  engine.addDefaultKey("configuration", configuration);

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
  List<String> input;
  List<String> output;
  List<String> command;
  List<String> message;

  unsigned int finishedDependencies;
  Map<Rule*, void*> dependencies;
  Map<Rule*, void*> propagations;

  bool rebuild;

  Process process;

  Rule() : finishedDependencies(0), rebuild(false) {}

  bool startExecution(unsigned int& pid, bool clean, bool rebuild)
  {
    assert(finishedDependencies == dependencies.getSize());

    if(clean)
      goto clean;
    if(rebuild)
      goto build;

    // determine whether to build this rule
    for(Map<Rule*, void*>::Node* i = dependencies.getFirst(); i; i = i->getNext())
      if(i->key->rebuild)
      {
        // TODO: debug message
        goto build;
      }
    {
      long long minWriteTime = 0;
      for(const List<String>::Node* i = output.getFirst(); i; i = i->getNext())
      {
        const String& file = i->data;
        long long writeTime;
        if(!File::getWriteTime(file, writeTime))
        {
          // TODO: debug message
          goto build;
        }
        if(i == output.getFirst() || writeTime < minWriteTime)
          minWriteTime = writeTime;
      }
      for(const List<String>::Node* i = input.getFirst(); i; i = i->getNext())
      {
        const String& file = i->data;
        long long writeTime;
        if(!File::getWriteTime(file, writeTime))
        {
          // TODO: debug message
          goto build;
        }
        if(writeTime >= minWriteTime)
        {
          // TODO: debug message
          goto build;
        }
      }
    }

    // no rebuilding
    pid = 0;
    return true;
  
clean:

    // delete output files and directories
    for(const List<String>::Node* i = output.getFirst(); i; i = i->getNext())
    {
      if(File::exists(i->data))
      {
        File file;
        if(!file.unlink(i->data))
        {
          // TODO: error message
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

    if(command.isEmpty())
    {
      pid = 0;
      return true;
    }

    // create output directories
    for(const List<String>::Node* i = output.getFirst(); i; i = i->getNext())
      Directory::create(File::getDirname(i->data));

    pid = process.start(command);
    if(!pid)
    {
      fprintf(stderr, "%s\n", process.getErrno().getString().getData());
      return false;
    }
    return true;
  }

  bool finishExecution()
  {
    unsigned int exitCode = process.wait();
    return exitCode == 0;
  }
};

class Target
{
public:
  List<Rule> rules;
  bool active;

  Target() : active(false) {}
};

class RuleSet
{
public:
  List<Target> targets;
  List<Target*> activeTargets;

  unsigned int activeRules;
  unsigned int finishedRules;

  RuleSet() : activeRules(0), finishedRules(0) {}

  void resolveDependencies()
  {
    // generate outputToRule map
    Map<String, Rule*> outputToRule;
    for(List<Target>::Node* i = targets.getFirst(); i; i = i->getNext())
      for(List<Rule>::Node* j = i->data.rules.getFirst(); j; j = j->getNext())
      {
        Rule& rule = j->data;
        if(rule.output.isEmpty())
        { // TODO: warning
        }
        if(rule.command.isEmpty())
        { // TODO: warning
        }
        for(List<String>::Node* i = rule.output.getFirst(); i; i = i->getNext())
        {
          if(outputToRule.find(i->data))
          { // TODO: warning
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
        for(List<String>::Node* i = rule.input.getFirst(); i; i = i->getNext())
        {
          Rule* dependency = outputToRule.lookup(i->data);
          if(dependency)
          {
            if(dependency == &rule)
            {
              // TODO: warning
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
              rule.dependencies.append(dependency, 0);
            if(!dependency->propagations.find(&rule))
              dependency->propagations.append(&rule, 0);
          }
        }
      }
  }
  
  bool build(unsigned int maxParallelJobs, bool clean, bool rebuild)
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
          if(!rule->startExecution(pid, clean, rebuild))
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
      for(Map<Rule*, void*>::Node* i = rule->propagations.getFirst(); i; i = i->getNext())
      {
        Rule& rule = *i->key;
        assert(!rule.dependencies.isEmpty());
        ++rule.finishedDependencies;
        if(rule.finishedDependencies == rule.dependencies.getSize())
          pendingJobs.append(&rule);
      }
    } while(!runningJobs.isEmpty() || (!pendingJobs.isEmpty() && !failure));

    if(failure)
      return false;

    // unresolvable dependencies?
    if(finishedRules < activeRules)
    {
      // TODO: error message
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
    Target& target = ruleSet.targets.append();
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
        engine.enterKey(i->data);
        engine.addDefaultKey("file", i->data);
        engine.getKeys("input", rule.input, false);
        engine.getKeys("output", rule.output, false);
        engine.getKeys("command", rule.command, false);
        engine.getKeys("message", rule.message, false);
        engine.leaveKey();
      }
      engine.leaveKey();
    }

    // add rule for target file
    Rule& rule = target.rules.append();
    rule.target = &target;
    engine.getKeys("input", rule.input, false);
    engine.getKeys("output", rule.output, false);
    engine.getKeys("command", rule.command, false);
    engine.getKeys("message", rule.message, false);

    engine.leaveKey();
  }

  ruleSet.resolveDependencies();
  return ruleSet.build(jobs <= 0 ? (Process::getProcessorCount() - jobs) : jobs, clean, rebuild);
}

