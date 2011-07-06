
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <malloc.h>

#include "../libmare/Engine.h"
#include "../libmare/Tools/Words.h"
#include "../libmare/Tools/String.h"
#include "../libmare/Tools/File.h"
#include "../libmare/Tools/Directory.h"
#include "../libmare/Tools/Map.h"
#include "../libmare/Tools/Process.h"
#include "../libmare/Tools/Win32/getopt.h"

static const char* VERSION   = "0.1";
static const char* COPYRIGHT = "Copyright (C) 2011 Colin Graf";

static void errorHandler(void* userData, unsigned int line, const String& message)
{
  if(!userData)
    fprintf(stderr, "%s\n", message.getData());
  else if(line)
    fprintf(stderr, "%s:%u: error: %s\n", (const char*)userData, line, message.getData());
  else
    fprintf(stderr, "%s: %s\n", (const char*)userData, message.getData());
}

static void showVersion(bool andExit)
{
  printf("mare %s, the lightweight build system\n", VERSION);
  puts(COPYRIGHT);
  //printf("Author(s): %s\n", AUTHORS);
  puts("This program comes with ABSOLUTELY NO WARRANTY.");
  puts("This is free software, and you are welcome to redistribute it under certain ");
  puts("conditions.");
  if(andExit)
    exit(EXIT_SUCCESS);
}

static void showUsage(const char* executable)
{
  String basename = File::getBasename(String(executable, -1));
  showVersion(false);
  puts("");
  printf("Usage: %s [ -f <file> ] [ <options> ] [ config=<config> ] [ <target> ]", basename.getData());
  puts("        [ <variable>=<value> ]");
  puts("");
  puts("Options:");
  puts("");
  puts("    -f <file>, --file=<file>");
  puts("        Use <file> as a marefile.");
  puts("");
  puts("    config=<config>, --config=<config>");
  puts("        Build using configuration <config> as declared in the marefile (Debug ");
  puts("        and Release by default). Multiple configurations can be used by add ");
  puts("        adding config=<config> multiple times.");
  puts("");
  puts("    <target>, --target=<target>");
  puts("        Build <target> as declared in the marefile. Multiple targets can be ");
  puts("        used.");
  puts("");
  puts("    <variable>=<value>, --<variable>=<value>");
  puts("        Set any variable <variable> to <value>. This can be used to set");
  puts("        various options with a meaning defined by the marefile.");
  puts("");
  puts("    -d");
  puts("        Print debugging information while processing normally.");
  puts("");
  puts("    -h, --help");
  puts("        Display this help message.");
  puts("");
  puts("    -v, --version");
  puts("        Display version information.");
  puts("");
  exit(EXIT_SUCCESS);
}

static void showHelp(const char* executable)
{
  String basename = File::getBasename(String(executable, -1));
  fprintf(stderr, "Type '%s --help' for help\n", basename.getData());
  exit(EXIT_FAILURE);
}

static bool buildFile(Engine& engine, const String& inputFile, const List<String>& inputConfigs, const List<String>& inputTargets);
static bool buildConfigurations(Engine& engine, const String& inputFile, const List<String>& inputConfigs, const List<String>& inputTargets);
static bool buildConfiguration(Engine& engine, const String& inputFile, const String& configuration, const List<String>& inputTargets);
static bool buildTargets(Engine& engine, const List<String>& inputTargets);

int main(int argc, char* argv[])
{
  Map<String, String> userArgs;
  String inputFile("Marefile");
  bool showHelp = false;
  bool showDebug = false;

  // parse args
  {
    int c, option_index;
    static struct option long_options[] = {
      {"file", required_argument , 0, 'f'},
      {"help", no_argument , 0, 'h'},
      {"version", no_argument , 0, 'v'},
      {0, 0, 0, 0}
    };

    // find all user arguments
    char** nargv = (char**)alloca(sizeof(char*) * argc);
    int nargc = 1;
    nargv[0] = argv[0];
    for(int i = 1; i < argc; ++i)
    {
      if(strncmp(argv[i], "--", 2) != 0)
        nargv[nargc++] = argv[i];
      else
      {
        const char* arg = argv[i] + 2;
        for(struct option* opt = long_options; opt->name; ++opt)
          if(strcmp(arg, opt->name) == 0)
          {
            nargv[nargc++] = argv[i];
            goto nextarg;
          }
        const char* sep = strchr(arg, '=');
        if(sep)
          userArgs.append(String(arg, sep - arg), String(sep + 1, -1));
        else
          userArgs.append(String(arg, -1), String());
      }
    nextarg:;
    }

    // parse normal arguments
    while((c = getopt_long(nargc, nargv, "c:df:hv", long_options, &option_index)) != -1)
      switch(c)
      {
      case 'd':
        showDebug = true;
        break;
      case 'f':
        inputFile = String(optarg, -1);
        break;
      case 'h':
        showHelp = true;
        break;
      case 'v':
        showVersion(true);
        break;
      default:
        ::showHelp(argv[0]);
        break;
      }
    while(optind < argc)
    {
      const char* arg = argv[optind++];
      const char* sep = strchr(arg, '=');
      if(sep)
        userArgs.append(String(arg, sep - arg), String(sep + 1, -1));
      else
        userArgs.append(String("target"), String(arg, -1));
    }
  }

  {
    Engine engine;
    if(!engine.load(inputFile, errorHandler, (void*)inputFile.getData()))
    {
      if(showHelp)
        showUsage(argv[0]);

      return EXIT_FAILURE;
    }

    // show help only?
    if(showHelp)
    {
      if(engine.enterKey("help"))
      {
        List<String> help;
        engine.getKeys(help);
        for(List<String>::Node* node = help.getFirst(); node; node = node->getNext())
          puts(node->data.getData());
        return EXIT_SUCCESS;
      }
      else
        showUsage(argv[0]);
    }

    // add default rules and stuff
    engine.addDefaultKey("CC", "gcc");
    engine.addDefaultKey("CXX", "g++");
    engine.enterDefaultKey("configurations");
      engine.addResolvableKey("Debug");
      engine.addResolvableKey("Release");
    engine.leaveKey();
    engine.addDefaultKey("targets");
    engine.addDefaultKey("buildDir", "$(configuration)");
    engine.enterDefaultKey("cppCompile");
      engine.addResolvableKey("ofile", "$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file)))");
      engine.addResolvableKey("dfile", "$(patsubst %.o,%.d,$(ofile))");
      engine.addResolvableKey("input", "$(file) $(filter-out %.o: \\,$(readfile $(dfile)))");
      engine.addResolvableKey("output", "$(ofile) $(dfile)");
      engine.addResolvableKey("command", "$(CXX) -MMD -o $(ofile) -c $(file) $(CXXFLAGS) $(patsubst %,-D%,$(defines)) $(patsubst %,-I%,$(includePaths))");
      engine.addResolvableKey("message", "$(file)");
    engine.leaveKey();
    engine.enterDefaultKey("cppLink");
      engine.addResolvableKey("input", "$(foreach file,$(filter %.c%,$(files)),$(buildDir)/$(patsubst %.%,%.o,$(subst ../,,$(file))))");
      engine.addResolvableKey("output", "$(buildDir)/$(target)");
      engine.addResolvableKey("command", "$(CXX) -o $(output) $(input) $(LDFLAGS) $(patsubst %,-L%,$(libPaths)) $(patsubst %,-l%,$(libs))");
      engine.addResolvableKey("message", "Linking $(target)...");
    engine.leaveKey();

    // add user arguments
    engine.enterUnnamedKey();
    for(Map<String, String>::Node* i = userArgs.getFirst(); i; i = i->getNext())
    {
      
      if(i->key == "config")
        i->key = "configuration";
      engine.addDefaultKey(i->key, i->data);
    }

    // get targets and configurations to build
    List<String> inputConfigs, inputTargets;
    engine.getKeys("configuration", inputConfigs);
    engine.getKeys("target", inputTargets);

    // build 
    if(!buildFile(engine, inputFile, inputConfigs, inputTargets))
      return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

static bool buildFile(Engine& engine, const String& inputFile, const List<String>& inputConfigs, const List<String>& inputTargets)
{
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
      bool result = buildConfiguration(engine, inputFile, firstConfiguration, inputTargets);
      engine.leaveKey();
      return result;
    }
    else
    {
      errorHandler((void*)inputFile.getData(), 0, "cannot find any configurations");
      return false;
    }
  }

  bool result = buildConfigurations(engine, inputFile, inputConfigs, inputTargets);
  engine.leaveKey();
  return result;
}

static bool buildConfigurations(Engine& engine, const String& inputFile, const List<String>& inputConfigs, const List<String>& inputTargets)
{
  for(const List<String>::Node* i = inputConfigs.getFirst(); i; i = i->getNext())
  {
    if(!engine.enterKey(i->data))
    {
      String message;
      message.format(256, "cannot find configuration \"%s\"", i->data.getData());
      errorHandler((void*)inputFile.getData(), 0, message);
      return false;
    }
    if(!buildConfiguration(engine, inputFile, i->data, inputTargets))
    {
      engine.leaveKey();
      return false;
    }
    engine.leaveKey();
  }
  return true;
}

static bool buildConfiguration(Engine& engine, const String& inputFile, const String& configuration, const List<String>& inputTargets)
{
  engine.addDefaultKey("configuration", configuration);

  if(!engine.enterKey("targets"))
  {
    assert(false);
    return false;
  }

  if(inputTargets.isEmpty())
  { // build all
    List<String> inputTargets;
    engine.getKeys(inputTargets);
    if(inputTargets.isEmpty())
    {
      errorHandler((void*)inputFile.getData(), 0, "cannot find any targets");
      return false;
    }
    return buildTargets(engine, inputTargets);
  }

  for(const List<String>::Node* node = inputTargets.getFirst(); node; node = node->getNext())
  {
    if(!engine.enterKey(node->data))
    {
      String message;
      message.format(256, "cannot find target \"%s\"", node->data.getData());
      errorHandler((void*)inputFile.getData(), 0, message);
      return false;
    }
    engine.leaveKey();
  }
  return buildTargets(engine, inputTargets);
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

  bool startExecution(unsigned int& pid)
  {
    assert(finishedDependencies == dependencies.getSize());

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
  
    //
  build:
    rebuild = true;

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
      errorHandler(0, 0, process.getErrno().getString());
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
  
  bool build(unsigned int maxParallelJobs)
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
          continue; // wtf?
        rule = job->data;
        runningJobs.remove(job);
        if(!rule->finishExecution())
          failure = true;
      }

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

static bool buildTargets(Engine& engine, const List<String>& inputTargets)
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
  return ruleSet.build(Process::getProcessorCount());
}
