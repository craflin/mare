
#include <malloc.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdio>
#include <cstring>
#endif

#include "Assert.h"
#include "Process.h"
#include "Map.h"
#include "File.h"
#include "Word.h"
#ifdef _WIN32
#include "Array.h"
#else
#include "Error.h"
#endif

#ifdef _WIN32
static Array<HANDLE> runningProcessHandles;
#else
static Map<pid_t, Process*> runningProcesses;
#endif

Process::Process()
{
#ifdef _WIN32
  ASSERT(sizeof(hProcess) >= sizeof(HANDLE));
  hProcess = INVALID_HANDLE_VALUE;
#else
  pid = 0;
  exitCode = 1;
#endif
}

Process::~Process()
{
#ifdef _WIN32
  ASSERT(hProcess ==  INVALID_HANDLE_VALUE);
  if(hProcess != INVALID_HANDLE_VALUE)
  {
    CloseHandle((HANDLE)hProcess);

    int index = runningProcessHandles.find(hProcess);
    if(index >= 0)
      runningProcessHandles.remove(index);
  }
#else
  ASSERT(pid ==  0);
#endif
}

bool Process::isRunning() const
{
#ifdef _WIN32
  return hProcess != INVALID_HANDLE_VALUE;
#else
  return pid != 0;
#endif
}

unsigned int Process::start(const String& rawCommandLine)
{
  List<Word> command;
  Word::split(rawCommandLine, command);

#ifdef _WIN32
  struct Executable
  {
    static bool fileComplete(const String& searchName, bool testExtensions, String& result)
    {
      if(File::exists(searchName))
      {
        result = searchName;
        return true;
      }
      if(testExtensions)
      {
        String testPath = searchName;
        testPath.append(".exe");
        if(File::exists(testPath))
        {
          result = testPath;
          return true;
        }
        testPath.setLength(searchName.getLength());
        testPath.append(".com");
        if(File::exists(testPath))
        {
          result = testPath;
          return true;
        }
      }
      return false;
    }

    static const List<String>& getPathEnv()
    {
      static List<String> searchPaths;
      static bool loaded = false;
      if(!loaded)
      {
        char* pathVar = (char*)alloca(32767);
        GetEnvironmentVariable("PATH", pathVar, 32767);
        for(const char* str = pathVar; *str;)
        {
          const char* end = strchr(str, ';');
          if(end)
          {
            if(end > str)
              searchPaths.append(String(str, end - str));
            ++end;
            str = end;
          }
          else
          {
            searchPaths.append(String(str, -1));
            break;
          }
        }
        loaded = true;
      }
      return searchPaths;
    }

    static bool resolveSymlink(const String& fileName, String& result)
    {
      String cygwinRoot = File::getDirname(File::getDirname(fileName));
      result = fileName;
      bool success = false;
      for(;;)
      {
        File file;
        if(!file.open(result))
          return success;
        const int len = 12 + MAX_PATH * 2 + 2;
        char buffer[len];
        int i = file.read(buffer, len);
        if(i < 12 || strncmp(buffer, "!<symlink>ÿþ", 12) != 0)
          return success;
        i &= ~1;
        wchar_t* wdest = (wchar_t*)(buffer + 12);
        wdest[(i - 12) >> 1] = 0;
        String dest;
        dest.format(i - 12, "%S", wdest);
        if(strncmp(dest.getData(), "/usr/bin/", 9) == 0)
        {
          result = cygwinRoot;
          result.append(dest.substr(4));
        }
        else if(dest.getData()[0] == '/')
        {
          result = cygwinRoot;
          result.append(dest);
        }
        else
        {
          result = File::getDirname(result);
          result.append('/');
          result.append(dest);
        }
        success = true;
      }
      return false;
    }

    static String find(const String& program)
    {
      String result = program;
      bool testExtensions = File::getExtension(program).isEmpty();
      // check whether the given path is absolute
      if(program.getData()[0] == '/' || (program.getLength() > 2 && program.getData()[1] == ':'))
      { // absolute
        fileComplete(program, testExtensions, result);
      }
      else
      { // try each search path
        const List<String>& searchPaths = getPathEnv();
        for(const List<String>::Node* i = searchPaths.getFirst(); i; i = i->getNext())
        {
          String testPath = i->data;
          testPath.append('\\');
          testPath.append(program);
          if(fileComplete(testPath, testExtensions, result))
          {
            if(strncmp(program.getData(), "../", 3) == 0 || strncmp(program.getData(), "..\\", 3) == 0)
              result = File::simplifyPath(result);
            break;
          }
        }
      }
      return result;
    }
  };

  String program, programPath;
  static Map<String, String> cachedProgramPaths;
  bool cachedProgramPath = false;
  if(command.isEmpty())
    cachedProgramPath = true;
  else
  {
    program = command.getFirst()->data;
    const Map<String, String>::Node* i = cachedProgramPaths.find(program);
    if(i)
    {
      programPath = i->data;
      cachedProgramPath = true;
    }
    else
      programPath = Executable::find(program);
  }

  String commandLine;
  if(!command.isEmpty())
  {
    if(strncmp(command.getFirst()->data.getData(), "../", 3) == 0 || strncmp(command.getFirst()->data.getData(), "..\\", 3) == 0)
      command.getFirst()->data = programPath;
    Word::append(command, commandLine);
  }
  commandLine.setCapacity(commandLine.getLength()); // enforce detach

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  if(!CreateProcess(programPath.getData(), (char*)commandLine.getData(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
  {
    DWORD lastError = GetLastError();
    if(!programPath.isEmpty())
    {
      String resolvedSymlink;
      if(Executable::resolveSymlink(programPath, resolvedSymlink))
      {
        programPath = resolvedSymlink;
        if(CreateProcess(programPath.getData(), (char*)commandLine.getData(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
          goto success;
        else
          lastError = GetLastError();
      }
    }

    if(!cachedProgramPath)
      cachedProgramPaths.append(program, programPath);

    SetLastError(lastError);
    return 0;
  }
success:

  if(!cachedProgramPath)
    cachedProgramPaths.append(program, programPath);

  CloseHandle(pi.hThread);

  ASSERT(pi.hProcess);
  hProcess = pi.hProcess;
  runningProcessHandles.append(pi.hProcess);

  return pi.dwProcessId;
#else

  struct Executable
  {
    static const List<String>& getPathEnv()
    {
      static List<String> searchPaths;
      static bool loaded = false;
      if(!loaded)
      {
        char* pathVar = getenv("PATH");
        for(const char* str = pathVar; *str;)
        {
          const char* end = strchr(str, ':');
          if(end)
          {
            if(end > str)
              searchPaths.append(String(str, end - str));
            ++end;
            str = end;
          }
          else
          {
            searchPaths.append(String(str, -1));
            break;
          }
        }
        loaded = true;
      }
      return searchPaths;
    }

#ifdef __CYGWIN__
    static bool fileComplete(const String& searchName, bool testExtensions, String& result)
    {
      if(File::exists(searchName))
      {
        result = searchName;
        return true;
      }
      if(testExtensions)
      {
        String testPath = searchName;
        testPath.append(".exe");
        if(File::exists(testPath))
        {
          result = testPath;
          return true;
        }
        testPath.setLength(searchName.getLength());
        testPath.append(".com");
        if(File::exists(testPath))
        {
          result = testPath;
          return true;
        }
      }
      return false;
    }
    static String find(const String& program)
    {
      String result = program;
      bool testExtensions = File::getExtension(program).isEmpty();
      // check whether the given path is absolute
      if(program.getData()[0] == '/')
      { // absolute
        fileComplete(program, testExtensions, result);
      }
      else
      { // try each search path
        const List<String>& searchPaths = Executable::getPathEnv();
        for(const List<String>::Node* i = searchPaths.getFirst(); i; i = i->getNext())
        {
          String testPath = i->data;
          testPath.append('/');
          testPath.append(program);
          if(fileComplete(testPath, testExtensions, result))
            break;
        }
      }
      return result;
    }
#else
    static String find(const String& program)
    {
      String result = program;
      // check whether the given path is absolute
      if(program.getData()[0] == '/')
      { // absolute
        return result;
      }
      else
      { // try each search path
        const List<String>& searchPaths = Executable::getPathEnv();
        for(const List<String>::Node* i = searchPaths.getFirst(); i; i = i->getNext())
        {
          String testPath = i->data;
          testPath.append('/');
          testPath.append(program);
          if(File::exists(testPath))
          {
            result = testPath;
            break;
          }
        }
      }
      return result;
    }
#endif
  };

  String program, programPath;
  static Map<String, String> cachedProgramPaths;
  if(!command.isEmpty())
  {
    program = command.getFirst()->data;
    const Map<String, String>::Node* i = cachedProgramPaths.find(program);
    if(i)
      programPath = i->data;
    else
    {
      programPath = Executable::find(program);
      cachedProgramPaths.append(program, programPath);
    }
  }

  const char** argv = (const char**)alloca(sizeof(const char*) * (command.getSize() + 1));
  int i = 0;
  for(const List<Word>::Node* j = command.getFirst(); j; j = j->getNext())
    argv[i++] = j->data.getData();
  argv[i] = 0;

  int r = vfork();
  if(r == -1)
    return 0;
  else if(r != 0) // parent
  {
    pid = r;
    runningProcesses.append(pid, this);
    return r;
  }
  else // child
  {
    const char* executable = programPath.getData();
    if(execv(executable, (char* const*)argv) == -1)
    {
      fprintf(stderr, "%s: %s\n", executable, Error::getString().getData());
      _exit(EXIT_FAILURE);
    }
    ASSERT(false); // unreachable
    return 0;
  }
#endif
}

unsigned int Process::join()
{
#ifdef _WIN32
  if(hProcess == INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return 0;
  }
  DWORD exitCode = 0;
  GetExitCodeProcess(hProcess, &exitCode);
  CloseHandle((HANDLE)hProcess);
  hProcess = INVALID_HANDLE_VALUE;
  return exitCode;
#else
  if(!pid)
  {
    errno = EINVAL;
    return 0;
  }
  pid = 0;
  return exitCode;
#endif
}

unsigned int Process::waitOne()
{
#ifdef _WIN32
  if(runningProcessHandles.isEmpty())
  {
    SetLastError(ERROR_NOT_READY);
    return 0;
  }
  DWORD index = WaitForMultipleObjects(runningProcessHandles.getSize(), runningProcessHandles.getFirst(), FALSE, INFINITE);
  if(index == WAIT_FAILED)
    return 0;
  index -= WAIT_OBJECT_0;
  ASSERT(index >= 0 && index < runningProcessHandles.getSize());

  HANDLE handle = runningProcessHandles.getFirst()[index];
  runningProcessHandles.remove(index);

  return GetProcessId(handle);
#else
  int status;
  pid_t pid = wait(&status);
  if(pid == -1)
    return 0;
  Map<pid_t, Process*>::Node* i = runningProcesses.find(pid);
  if(i)
  {
    i->data->exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    runningProcesses.remove(i);
  }
  return pid;
#endif
}

unsigned  int Process::getProcessorCount()
{
#if defined(_WIN32)
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return si.dwNumberOfProcessors;
//#elif defined(__linux)
#else
  return sysconf(_SC_NPROCESSORS_CONF);
//#else
  //return 1;
#endif
}
