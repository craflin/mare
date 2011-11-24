
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <malloc.h>

#include "Engine.h"
#include "Tools/File.h"
#include "Tools/Directory.h"
#include "Tools/Error.h"
#ifdef _WIN32
#include "Tools/Win32/getopt.h"
#else
#include <getopt.h>
#endif

#include "Builder.h"
#include "Vcxproj.h"

static const char* VERSION   = "0.3";
static const char* COPYRIGHT = "Copyright (C) 2011 Colin Graf";

static void errorHandler(void* userData, const String& file, int line, const String& message)
{
  if(line < 0)
    fprintf(stderr, "%s: %s\n", Error::program, message.getData());
  else
  {
    if(line > 0)
      fprintf(stderr, "%s:%u: error: %s\n", file.getData(), line, message.getData());
    else
      fprintf(stderr, "%s: %s\n", file.getData(), message.getData());
  }
}

static void showVersion(bool andExit)
{
  printf("mare %s, the lightweight build system\n", VERSION);
  puts(COPYRIGHT);
  //printf("Author(s): %s\n", AUTHORS);
  puts("This program comes with ABSOLUTELY NO WARRANTY.");
  puts("This is free software, and you are welcome to redistribute it under certain");
  puts("conditions.");
  if(andExit)
    exit(EXIT_SUCCESS);
}

static void showUsage(const char* executable)
{
  String basename = File::getBasename(String(executable, -1));
  showVersion(false);
  puts("");
  printf("Usage: %s [ -f <file> ] [ <options> ] [ config=<config> ] [ <target> ]\n", basename.getData());
  puts("        [ platform=<platform> ] [ <variable>=<value> ] [ clean | rebuild ]");
  puts("");
  puts("Options:");
  puts("");
  puts("    -f <file>, --file=<file>");
  puts("        Use <file> as a marefile.");
  puts("");
  puts("    --vcxproj [ 2010 ]");
  puts("        Generate a .sln and .vcxproj files for Visual Studio 2010 from the");
  puts("        marefile.");
  puts("");
  puts("    config=<config>, --config=<config>");
  puts("        Build using configuration <config> as declared in the marefile (Debug");
  puts("        and Release by default). Multiple configurations can be used by adding");
  puts("        config=<config> multiple times.");
  puts("");
  puts("    <target>, target=<target>, --target=<target>");
  puts("        Build <target> as declared in the marefile. Multiple targets can be");
  puts("        used.");
  puts("");
  puts("    platform=<config>, --platform=<config>");
  puts("        Build the target for platform <platform>. By default the native");
  puts("        build platform of the mare executable or the native platform of a");
  puts("        generator (e.g. \"Win32\" for vcxproj) will be used. Multiple platforms");
  puts("        can be used by adding platform=<platform> multiple times. Available");
  puts("        platforms can be declared in the mare.");
  puts("");
  puts("    <variable>=<value>, --<variable>[=<value>]");
  puts("        Set any variable <variable> to <value>. This can be used to set");
  puts("        various options with a meaning defined by the marefile.");
  puts("");
  puts("    clean, --clean");
  puts("        Delete all output files instead of building the selected target.");
  puts("");
  puts("    rebuild, --rebuild");
  puts("        Rebuild all output files of the selected target.");
  puts("");
  puts("    -C <dir>, --directory=<dir>");
  puts("        Change working directory to <dir> before doing anything else.");
  puts("");
  puts("    -d");
  puts("        Print debugging information while processing normally.");
  puts("");
  puts("    -j <jobs>");
  puts("        Use <jobs> processes in parallel for building alle targets. The default");
  puts("        value for <jobs> is the number of processors on the host system.");
  puts("");
  puts("    --ignore-dependencies");
  puts("        Do not respect dependencies between build targets.");
  puts("");
  puts("    -h, --help");
  puts("        Display this help message or a help message declared in the marefile.");
  puts("");
  puts("    -v, --version");
  puts("        Display version information.");
  puts("");
  exit(EXIT_SUCCESS);
}

static void showHelp(const char* executable)
{
  fprintf(stderr, "Type '%s --help' for help\n", executable);
  exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
  Error::program = argv[0];
  Map<String, String> userArgs;
  List<String> inputPlatforms, inputConfigs, inputTargets;
  String inputFile("Marefile"), inputDir;
  bool showHelp = false;
  bool showDebug = false;
  bool clean = false;
  bool rebuild = false;
  bool ignoreDependencies = false;
  int jobs = 0;
  int generateVcxproj = 0;

  // parse args
  {
    int c, option_index;
    static struct option long_options[] = {
      {"file", required_argument , 0, 'f'},
      {"help", no_argument , 0, 'h'},
      {"version", no_argument , 0, 'v'},
      {"directory", required_argument , 0, 'C'},
      {"clean", no_argument , 0, 0},
      {"rebuild", no_argument , 0, 0},
      {"ignore-dependencies", no_argument , 0, 0},
      {"vcxproj", optional_argument , 0, 0},
      {0, 0, 0, 0}
    };

    // find and remove all user arguments
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
        {
          String key(arg, sep - arg);
          String val(sep + 1, -1);
          if(key == "platform")
            inputPlatforms.append(val);
          else if(key == "config")
            inputConfigs.append(val);
          else if(key == "target")
            inputTargets.append(val);
          else
            userArgs.append(key, val);
        }
        else
          userArgs.append(String(arg, -1), String());
      }
    nextarg:;
    }
    argc = nargc;
    argv = nargv;

    // parse normal arguments
    while((c = getopt_long(argc, argv, "C:df:hj:v", long_options, &option_index)) != -1)
      switch(c)
      {
      case 0:
        {
          String opt(long_options[option_index].name, -1);
          if(opt == "vcxproj")
          {
            generateVcxproj = 2010;
            if(optarg)
            {
              if(strcmp(optarg, "2010") == 0)
                generateVcxproj = 2010;
              else // unknown version
                ::showHelp(argv[0]);
            }
          }
          else if(opt == "clean")
            clean = true;
          else if(opt == "rebuild")
            rebuild = true;
          else if(opt == "ignore-dependencies")
            ignoreDependencies = true;
        }
        break;
      case 'C':
        inputDir = String(optarg, -1);
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
      case 'j':
        jobs = atoi(optarg);
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
      {
        String key(arg, sep - arg);
        String val(sep + 1, -1);
        if(key == "platform")
          inputPlatforms.append(val);
        else if(key == "config")
          inputConfigs.append(val);
        else if(key == "target")
          inputTargets.append(val);
        else
          userArgs.append(key, val);
      }
      else
      {
        String target(arg, -1);
        if(target == "clean")
          clean = true;
        else if(target == "rebuild")
          rebuild = true;
        else
          inputTargets.append(target);
      }
    }
  }

  // change working directory?
  if(!inputDir.isEmpty())
  {
    if(!Directory::change(inputDir))
    {
      // TODO: error message
      return EXIT_FAILURE;
    }
  }

  // start the engine
  {
    Engine engine(errorHandler, 0);
    if(!engine.load(inputFile))
    {
      if(showHelp)
        showUsage(argv[0]);

      return EXIT_FAILURE;
    }

    // show help only?
    if(showHelp)
    {
      engine.enterRootKey();
      if(engine.enterKey("help"))
      {
        List<String> help;
        engine.getText(help);
        for(List<String>::Node* node = help.getFirst(); node; node = node->getNext())
          puts(node->data.getData());
        return EXIT_SUCCESS;
      }
      else
        showUsage(argv[0]);
    }

    // generate vcxproj mode?
    if(generateVcxproj)
    {
      Vcxproj vcxprog(engine, generateVcxproj);
      if(!vcxprog.generate(userArgs))
        return EXIT_FAILURE;
      return EXIT_SUCCESS;
    }

    // direct build
    {
      Builder builder(engine, inputPlatforms, inputConfigs, inputTargets, showDebug, clean, rebuild, jobs, ignoreDependencies);
      if(!builder.build(userArgs))
        return EXIT_FAILURE;
      return EXIT_SUCCESS;
    }
  }
}
