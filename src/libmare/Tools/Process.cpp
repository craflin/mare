
#include <cassert>
#ifdef _WIN32
#include <windows.h>
#include <malloc.h>
#endif

#include "Process.h"
#ifdef _WIN32
#include "Array.h"
#include "Map.h"
#include "File.h"
#endif

#ifdef _WIN32
static Array<HANDLE> runningProcessHandles;
#endif

Process::Process()
{
#ifdef _WIN32
  assert(sizeof(hProcess) >= sizeof(HANDLE));
  hProcess = INVALID_HANDLE_VALUE;
#endif
}

Process::~Process()
{
#ifdef _WIN32
  if(hProcess != INVALID_HANDLE_VALUE)
  {
    CloseHandle((HANDLE)hProcess);

    int index = runningProcessHandles.find(hProcess);
    assert(index >= 0);
    runningProcessHandles.remove(index);
  }
#endif
}

unsigned int Process::start(const List<String>& command)
{
#ifdef _WIN32

  struct Executable
  {
    static bool Executable::fileExists(const String& file)
    {
      WIN32_FIND_DATAA wfd;
      HANDLE hFind = FindFirstFileA(file.getData(), &wfd);
      if(hFind == INVALID_HANDLE_VALUE) 
        return false;
      bool isDir = (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY;
      FindClose(hFind);
      return isDir;
    }

    static bool Executable::fileComplete(const String& searchName, bool testExtensions, String& result)
    {
      if(fileExists(searchName))
      {
        result = searchName;
        return true;
      }
      if(testExtensions)
      {
        String testPath = searchName;
        testPath.append(".exe");
        if(fileExists(testPath))
        {
          result = testPath;
          return true;
        }
        testPath.setLength(searchName.getLength());
        testPath.append(".com");
        if(fileExists(testPath))
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
        const List<String>& searchPaths = Executable::getPathEnv();
        for(const List<String>::Node* i = searchPaths.getFirst(); i; i = i->getNext())
        {
          String testPath = i->data;
          testPath.append('\\');
          testPath.append(program);
          if(fileComplete(testPath, testExtensions, result))
            break;
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

  const List<String>::Node* i = command.getFirst();
  String commandLine;
  if(i)
  {
    commandLine.append(i->data);
    for(i = i->getNext(); i; i = i->getNext())
    {
      commandLine.append(' ');
      commandLine.append(i->data);
    }
  }
  commandLine.setCapacity(commandLine.getLength()); // enforce detach

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  if(!CreateProcess(programPath.getData(), (char*)commandLine.getData(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
  {
    if(!programPath.isEmpty())
    {
      String resolvedSymlink;
      if(Executable::resolveSymlink(programPath, resolvedSymlink))
      {
        programPath = resolvedSymlink;
        if(CreateProcess(programPath.getData(), (char*)commandLine.getData(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
          goto success;
      }
    }

    if(!cachedProgramPath)
      cachedProgramPaths.append(program, programPath);

    error = GetLastError();
    return 0;
  }
success:

  if(!cachedProgramPath)
    cachedProgramPaths.append(program, programPath);

  CloseHandle(pi.hThread);

  assert(pi.hProcess);
  hProcess = pi.hProcess;
  runningProcessHandles.append(pi.hProcess);

  return pi.dwProcessId;
#endif
}

unsigned int Process::wait()
{
#ifdef _WIN32
  if(hProcess == INVALID_HANDLE_VALUE)
    return 0;
  DWORD exitCode = 0;
  GetExitCodeProcess(hProcess, &exitCode);

  CloseHandle((HANDLE)hProcess);

  int index = runningProcessHandles.find(hProcess);
  assert(index >= 0);
  runningProcessHandles.remove(index);

  hProcess = INVALID_HANDLE_VALUE;

  return exitCode;
#endif
}

unsigned int Process::waitOne()
{
#ifdef _WIN32
  if(runningProcessHandles.isEmpty())
    return 0;
  DWORD index = WaitForMultipleObjects(runningProcessHandles.getSize(), runningProcessHandles.getFirst(), FALSE, INFINITE);
  if(index == WAIT_FAILED)
    return 0;
  index -= WAIT_OBJECT_0;
  assert(index >= 0 && index < runningProcessHandles.getSize());

  HANDLE handle = runningProcessHandles.getFirst()[index];

  return GetProcessId(handle);
#endif
}

unsigned  int Process::getProcessorCount()
{
#ifdef _WIN32
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return si.dwNumberOfProcessors;
#elif defined(__linux__)
  return sysconf(_SC_NPROCESSORS_CONF);
#else
  return 1;
#endif
}
