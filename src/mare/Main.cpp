
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

#include "Mare.h"
#include "Make.h"
#include "Vcxproj.h"
#include "Vcproj.h"
#include "CodeLite.h"
#include "CodeBlocks.h"
#include "CMake.h"
#include "NetBeans.h"
#include "JsonDb.h"

static const char* VERSION   = "0.3";
static const char* COPYRIGHT = "Copyright (C) 2011-2013 Colin Graf";

static void errorHandler(void* userData, const String& file, int line, const String& message)
{
  if(line < 0)
    fprintf(stderr, "%s: %s\n", (const char*)userData, message.getData());
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
  puts("        Use <file> as input marefile.");
  puts("        The default value of <file> is \"Marefile\".");
  puts("");
  puts("    --vcxproj[=<version>]");
  puts("        Convert the marefile into a .sln and .vcxproj files for Visual Studio.");
  puts("        <version> can be set to 2010, 2012, 2013, 2015 or 2017.");
  puts("");
  puts("    --vcproj[=<version>]");
  puts("        Convert the marefile into a .sln and .vcproj files for Visual Studio.");
  puts("        <version> can be set to 2008.");
  puts("");
  puts("    --make");
  puts("        Try to translate the marefile into a Makefile. (experimental)");
  puts("");
  puts("    --codelite");
  puts("        Try to translate the marefile into a .workspace and .project files for");
  puts("        CodeLite. (experimental)");
  puts("");
  puts("    --codeblocks");
  puts("        Try to translate the marefile into a .workspace and .cbp files for");
  puts("        Code::Blocks. (experimental)");
  puts("");
  puts("    --cmake");
  puts("        Try to translate the marefile into a CMakeLists.txt files for cmake.");
  puts("        (experimental)");
  puts("");
  puts("    --netbeans");
  puts("        Generate project files for NetBeans. (experimental)");
  puts("");
  puts("    --jsondb");
  puts("        Generate JSON compilation database.");
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
  Map<String, String> userArgs;
  List<String> inputPlatforms, inputConfigs, inputTargets;
  String inputFile("Marefile"), inputDir;
  bool showHelp = false;
  bool showDebug = false;
  bool clean = false;
  bool rebuild = false;
  bool ignoreDependencies = false;
  int jobs = 0;
  bool generateMake = false;
  int generateVcxproj = 0;
  int generateVcproj = 0;
  bool generateCodeLite = false;
  bool generateCodeBlocks = false;
  bool generateCMake = false;
  bool generateNetBeans = false;
  bool generateJsonDb = false;

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
      {"make", no_argument , 0, 0},
      {"vcxproj", optional_argument , 0, 0},
      {"vcproj", optional_argument , 0, 0},
      {"codelite", no_argument , 0, 0},
      {"codeblocks", no_argument , 0, 0},
      {"cmake", no_argument , 0, 0},
      {"netbeans", no_argument , 0, 0},
      {"jsondb", no_argument , 0, 0},
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
        size_t arglen = strlen(arg);
        const char* argarg = strchr(arg, '=');
        if(argarg)
          arglen = argarg - arg;
        for(struct option* opt = long_options; opt->name; ++opt)
          if(strncmp(arg, opt->name, arglen) == 0 && strlen(opt->name) == arglen)
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
          if(opt == "make")
            generateMake = true;
          else if(opt == "vcxproj")
          {
            generateVcxproj = 2010;
            if(optarg)
            {
              if(strcmp(optarg, "2010") == 0)
                generateVcxproj = 2010;
              else if(strcmp(optarg, "2012") == 0)
                generateVcxproj = 2012;
              else if(strcmp(optarg, "2013") == 0)
                generateVcxproj = 2013;
              else if (strcmp(optarg, "2015") == 0)
                generateVcxproj = 2015;
              else if(strcmp(optarg, "2017") == 0)
                generateVcxproj = 2017;
              else // unknown version
                ::showHelp(argv[0]);
            }
          }
          else if(opt == "vcproj")
          {
            generateVcproj = 2008;
            if(optarg)
            {
              if(strcmp(optarg, "2008") == 0)
                generateVcproj = 2008;
              else // unknown version
                ::showHelp(argv[0]);
            }
          }
          else if(opt == "codelite")
            generateCodeLite = true;
          else if(opt == "codeblocks")
            generateCodeBlocks = true;
          else if(opt == "cmake")
            generateCMake = true;
          else if(opt == "netbeans")
            generateNetBeans = true;
          else if(opt == "jsondb")
            generateJsonDb = true;
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
    Engine engine(errorHandler, argv[0]);
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

    // generate Makefile mode?
    if(generateMake)
    {
      Make make(engine);
      if(!make.generate(userArgs))
        return EXIT_FAILURE;
      return EXIT_SUCCESS;
    }

    // generate vcxproj mode?
    if(generateVcxproj)
    {
      Vcxproj vcxprog(engine, generateVcxproj);
      if(!vcxprog.generate(userArgs))
        return EXIT_FAILURE;
      return EXIT_SUCCESS;
    }

    // generate vcproj mode?
    if(generateVcproj)
    {
      Vcproj vcproj(engine, generateVcproj);
      if(!vcproj.generate(userArgs))
        return EXIT_FAILURE;
      return EXIT_SUCCESS;
    }

    // generate codelite mode?
    if(generateCodeLite)
    {
      CodeLite codeLite(engine);
      if(!codeLite.generate(userArgs))
        return EXIT_FAILURE;
      return EXIT_SUCCESS;
    }

    // generate codeblocks mode?
    if(generateCodeBlocks)
    {
      CodeBlocks codeBlocks(engine);
      if(!codeBlocks.generate(userArgs))
        return EXIT_FAILURE;
      return EXIT_SUCCESS;
    }

    // generate codeblocks mode?
    if(generateCMake)
    {
      CMake cMake(engine);
      if(!cMake.generate(userArgs))
        return EXIT_FAILURE;
      return EXIT_SUCCESS;
    }

    // generate codeblocks mode?
    if(generateNetBeans)
    {
      NetBeans netBeans(engine);
      if(!netBeans.generate(userArgs))
        return EXIT_FAILURE;
      return EXIT_SUCCESS;
    }

    // generate JSON compilation database mode?
    if(generateJsonDb)
    {
      JsonDb jsonDb(engine);
      if(!jsonDb.generate(userArgs))
        return EXIT_FAILURE;
      return EXIT_SUCCESS;
    }

    // direct build
    {
      Mare mare(engine, inputPlatforms, inputConfigs, inputTargets, showDebug, clean, rebuild, jobs, ignoreDependencies);
      if(!mare.build(userArgs))
        return EXIT_FAILURE;
      return EXIT_SUCCESS;
    }
  }
}
