
#ifdef _WIN32
#include <windows.h>
#else
#include <cstring>
#include <cerrno>
#endif

#include "Error.h"

String Error::getString()
{
#ifdef _WIN32
  String result;
  DWORD len = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) result.getData(256),
        256, NULL );
  if(len > 0)
    result.setLength(len);
  return result;
#else
  return String(strerror(errno), -1);
#endif
}
