
#include <cassert>
#ifdef _WIN32
#include <windows.h>
#endif

#include "Process.h"
#ifdef _WIN32
#include "Array.h"
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
  String commandLine;
  const List<String>::Node* i = command.getFirst();
  if(i)
  {
    commandLine = i->data;
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

  
  if(!CreateProcessA(NULL, (char*)commandLine.getData(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
  {
    error = GetLastError();
    return 0;
  }

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
