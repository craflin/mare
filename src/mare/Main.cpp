
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <malloc.h>

#include "../libmare/Engine.h"
#include "../libmare/Tools/File.h"
#include "../libmare/Tools/Win32/getopt.h"

#include "Builder.h"
#include "Vcxproj.h"

static const char* VERSION   = "0.1";
static const char* COPYRIGHT = "Copyright (C) 2011 Colin Graf";

static void errorHandler(void* userData, unsigned int line, const String& message)
{
  if(line)
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
  printf("Usage: %s [ -f <file> ] [ <options> ] [ config=<config> ] [ <target> ]", basename.getData());
  puts("        [ <variable>=<value> ]");
  puts("");
  puts("Options:");
  puts("");
  puts("    -f <file>, --file=<file>");
  puts("        Use <file> as a marefile.");
  puts("");
  puts("    --vcxproj 2010");
  puts("        Generate a .sln and .vcxproj files for Visual Studio 2010 from the");
  puts("        marefile.");
  puts("");
  puts("    config=<config>, --config=<config>");
  puts("        Build using configuration <config> as declared in the marefile (Debug");
  puts("        and Release by default). Multiple configurations can be used by add");
  puts("        adding config=<config> multiple times.");
  puts("");
  puts("    <target>, --target=<target>");
  puts("        Build <target> as declared in the marefile. Multiple targets can be");
  puts("        used.");
  puts("");
  puts("    <variable>=<value>, --<variable>[=<value>]");
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

int main(int argc, char* argv[])
{
  Map<String, String> userArgs;
  String inputFile("Marefile");
  bool showHelp = false;
  bool showDebug = false;
  int generateVcxproj = 0;

  // parse args
  {
    int c, option_index;
    static struct option long_options[] = {
      {"file", required_argument , 0, 'f'},
      {"help", no_argument , 0, 'h'},
      {"version", no_argument , 0, 'v'},
      {"vcxproj", required_argument , 0, 0},
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
        {
          String key(arg, sep - arg);
          if(key == "config")
            key = "configuration";
          userArgs.append(key, String(sep + 1, -1));
        }
        else
          userArgs.append(String(arg, -1), String());
      }
    nextarg:;
    }

    // parse normal arguments
    while((c = getopt_long(nargc, nargv, "c:df:hv", long_options, &option_index)) != -1)
      switch(c)
      {
      case 0:
        if(strcmp(long_options[option_index].name, "vcxproj") == 0)
        {
          generateVcxproj = 2010;
          if(optarg)
          {
            if(strcmp(long_options[option_index].name, "2010") == 0)
              generateVcxproj = 2010;
            else // unknown version
              ::showHelp(argv[0]);
          }
        }
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
    {
      const char* arg = argv[optind++];
      const char* sep = strchr(arg, '=');
      if(sep)
      {
        String key(arg, sep - arg);
        if(key == "config")
          key = "configuration";
        userArgs.append(key, String(sep + 1, -1));
      }
      else
        userArgs.append(String("target"), String(arg, -1));
    }
  }

  // start the engine
  {
    Engine engine(errorHandler, (void*)inputFile.getData());
    if(!engine.load(inputFile))
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

    // generate vcxproh mode?
    if(generateVcxproj)
    {
      Vcxproj vcxprog(engine, generateVcxproj);
      if(!vcxprog.generate(userArgs))
        return EXIT_FAILURE;
      return EXIT_SUCCESS;
    }

    // direct build
    {
      Builder builder(engine);
      if(!builder.build(userArgs))
        return EXIT_FAILURE;
      return EXIT_SUCCESS;
    }
  }
}
