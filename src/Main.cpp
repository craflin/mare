
#include <cstdlib>
#include <cstdio>
#include <cassert>

#include "Engine.h"
#include "Tools/Words.h"
#include "Tools/String.h"
#include "Tools/File.h"
#include "Tools/Directory.h"
#include "Tools/Map.h"
#include "Tools/Process.h"
#include "Tools/Win32/getopt.h"

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
  puts("This is free software, and you are welcome to redistribute it under certain conditions.");
  if(andExit)
    exit(EXIT_SUCCESS);
}

static void showUsage(const char* executable)
{
  String basename = File::getBasename(String(executable, -1));
  showVersion(false);
  puts("");
  printf("Usage: %s [ -c <config> ] [ -f <file> ] [ <options> ] [ <targets> ]\n", basename.getData());
  puts("");
  puts("Options:");
  puts("");
  puts("    -c <config>, --config=<config>");
  puts("        Use <config> as active configuration.");
  puts("");
  puts("    -d");
  puts("        Print debugging information while processing normally.");
  puts("");
  puts("    -f <file>, --file=<file>");
  puts("        Use <file> as a marefile.");
  puts("");
  puts("    -h, --help");
  puts("        Display this help message.");
  puts("");
  puts("    -v, --version");
  puts("        Display version information.");
  // TODO: you can set various options with a meaning defined by the marefile simply by adding variable=value or --variable=value to the command line
  puts("");
  exit(EXIT_SUCCESS);
}

static void showHelp(const char* executable)
{
  String basename = File::getBasename(String(executable, -1));
  fprintf(stderr, "Type '%s --help' for help\n", basename.getData());
  exit(EXIT_FAILURE);
}

static bool build(Engine& engine, const List<String>& inputTargets);

int main(int argc, char* argv[])
{
  String inputFile("Marefile");
  String inputConfig;
  List<String> inputTargets;
  bool showHelp = false;
  bool showDebug = false;

  // parse args
  {
    int c, option_index;
    static struct option long_options[] = {
      {"file", required_argument , 0, 'f'},
      {"config", required_argument , 0, 'c'},
      {"help", no_argument , 0, 'h'},
      {"version", no_argument , 0, 'v'},
      {0, 0, 0, 0}
    };
    while((c = getopt_long(argc, argv, "c:df:hv", long_options, &option_index)) != -1)
      switch(c)
      {
      case 'c':
        inputConfig = String(optarg, -1);
        break;
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
      inputTargets.append(String(argv[optind++], -1));
  }

  {
    Engine engine;
    if(!engine.load(inputFile, errorHandler, (void*)inputFile.getData()))
    {
      if(showHelp)
        showUsage(argv[0]);

      return EXIT_FAILURE;
    }
    engine.addDefaultVariable("CC", "gcc");
    engine.addDefaultVariable("CXX", "g++");

    // show help only?
    if(showHelp)
    {
      if(engine.enter("help"))
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

    // enter selected configuration
    {
      bool enteredConfiguration = true;
      if(!engine.enter("configurations"))
      {
        if(!inputConfig.isEmpty())
          enteredConfiguration = false;
      }
      else
      {
        if(inputConfig.isEmpty())
          inputConfig = engine.getFirstKey();
        if(!inputConfig.isEmpty() && !engine.enter(inputConfig))
          enteredConfiguration = false;
      } 
      if(!enteredConfiguration)
      {
        String message;
        message.format(256, "cannot find configuration \"%s\"", inputConfig.getData());
        errorHandler((void*)inputFile.getData(), 0, message);
        return EXIT_FAILURE;
      }
      engine.addDefaultVariable("configuration", inputConfig.isEmpty() ? String("default") : inputConfig);
    }

    // find targets
    {
      bool enteredTargets = true;
      if(!engine.enter("targets"))
        enteredTargets = false;
      else if(inputTargets.isEmpty())
      {
        engine.getKeys(inputTargets);
        if(inputTargets.isEmpty())
          enteredTargets = false;
      }
      else
      {
        bool failure = false;
        for(List<String>::Node* node = inputTargets.getFirst(); node; node = node->getNext())
        {
          if(!engine.enter(node->data))
          {
            String message;
            message.format(256, "cannot find target \"%s\"", node->data.getData());
            errorHandler((void*)inputFile.getData(), 0, message);
            failure = true;
          }
          else
            engine.leave();
        }
        if(failure)
          return EXIT_FAILURE;
      }
      if(!enteredTargets)
      {
        String message;
        message.format(256, "cannot find any targets");
        errorHandler((void*)inputFile.getData(), 0, message);
        return EXIT_FAILURE;
      }
    }

    // build targets
    if(!build(engine, inputTargets))
      return EXIT_FAILURE;
    /*
    {
      bool failure = false;
      for(List<String>::Node* node = inputTargets.getFirst(); node; node = node->getNext())
      {
        if(!engine.enter(node->data))
        {
          assert(false);
          continue;
        }
        if(!buildTarget(engine))
        {
          fprintf(stderr, "failed to build target \"%s\"\n", node->data.getData());
          failure = true;
        }
        engine.leave();
      }
      if(failure)
        return EXIT_FAILURE;
    }
    */

  }
  return EXIT_SUCCESS;
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

static bool build(Engine& engine, const List<String>& inputTargets)
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
    engine.enter(i->data);
    engine.addDefaultVariable("target", i->data);

    // add rule for each source file
    if(engine.enter("files"))
    {
      engine.getKeys(files);
      for(List<String>::Node* i = files.getFirst(); i; i = i->getNext())
      {
        Rule& rule = target.rules.append();
        rule.target = &target;
        engine.enter(i->data);
        engine.addDefaultVariable("file", i->data);
        engine.getKeys("input", rule.input, false);
        //rule.input.append(i->data);
        engine.getKeys("output", rule.output, false);
        engine.getKeys("command", rule.command, false);
        engine.getKeys("message", rule.message, false);
        engine.leave();
      }
      engine.leave();
    }

    // add rule for target file
    Rule& rule = target.rules.append();
    rule.target = &target;
    engine.getKeys("input", rule.input, false);
    engine.getKeys("output", rule.output, false);
    engine.getKeys("command", rule.command, false);
    engine.getKeys("message", rule.message, false);

    engine.leave();
  }

  ruleSet.resolveDependencies();
  return ruleSet.build(Process::getProcessorCount());
}
