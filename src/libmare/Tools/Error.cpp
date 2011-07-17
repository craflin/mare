
#ifdef _WIN32
#include <windows.h>
#else
#include <cstring>
#endif

#include "Error.h"
#include "String.h"

const char* Error::program;

String Error::getString() const
{
#ifdef _WIN32
  String result;
  DWORD len = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) result.getData(256),
        256, NULL );
  if(len > 0)
    result.setLength(len);
  return result;
#else
  return String(strerror(err), -1);
#endif
}
