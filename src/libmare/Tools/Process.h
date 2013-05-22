

#pragma once

#include "String.h"
#include "Map.h"

class Process
{
public:

  Process();
  ~Process();

  /**
  * Starts the execution of a process
  * @param command The command used to start the process. The first word in \c command should be a path to the executable. All other words in \c command are used as arguments for launching the process.
  * @return The process id of the newly started process or \c 0 if an errors occured
  */
  unsigned int start(const String& command);

  /**
  * Returns the running state of the process
  * @return \c true when the process is currently running and can be joined using \c join()
  */
  bool isRunning() const;

  unsigned int join();

  static unsigned int waitOne();

  static unsigned  int getProcessorCount();

  static String getArchitecture();

  /**
  * Returns the environment variables of the current process.
  * @return A map that contains the environment variables.
  */
  static const Map<String, String>& getEnvironmentVariables();

private:
#ifdef _WIN32
  void* hProcess;
#else
  unsigned int pid;
  unsigned int exitCode;
#endif
};
