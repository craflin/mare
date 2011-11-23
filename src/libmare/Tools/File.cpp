
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#else
#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "Assert.h"
#include "File.h"
#include "String.h"

File::File()
{
#ifdef _WIN32
  ASSERT(sizeof(void*) >= sizeof(HANDLE));
  fp = INVALID_HANDLE_VALUE;
#else
  fp = 0;
#endif
}

File::~File()
{
  close();
}

void File::close()
{
#ifdef _WIN32
  if(fp != INVALID_HANDLE_VALUE)
  {
    CloseHandle((HANDLE)fp);
    fp = INVALID_HANDLE_VALUE;
  }
#else
  if(fp)
  {
    fclose((FILE*)fp);
    fp = 0;
  }
#endif
}

bool File::unlink(const String& file)
{
#ifdef _WIN32
  if(!DeleteFile(file.getData()))
    return false;
#else
  if(::unlink(file.getData()) != 0)
    return false;
#endif
  return true;
}

bool File::open(const String& file, Flags flags)
{
#ifdef _WIN32
  if(fp != INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return false;
  }
  DWORD desiredAccess = 0, creationDisposition = 0;
  if(flags & writeFlag)
  {
    desiredAccess |= GENERIC_WRITE;
    creationDisposition |= CREATE_ALWAYS;
  }
  if(flags & readFlag)
  {
    desiredAccess |= GENERIC_READ;
    if(!(flags & writeFlag))
      creationDisposition |= OPEN_EXISTING;
  }
  fp = CreateFileA(file.getData(), desiredAccess, FILE_SHARE_READ, NULL, creationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
  if(fp == INVALID_HANDLE_VALUE)
    return false;
#else
  if(fp)
    return false;
  const char* mode = (flags & (writeFlag | readFlag)) == (writeFlag | readFlag) ? "w+" : (flags & writeFlag ? "w" : "r");
  fp = fopen(file.getData(), mode);
  if(!fp)
    return false;
#endif

  return true;
}

int File::read(char* buffer, int len)
{
#ifdef _WIN32
  DWORD i;
  if(!ReadFile((HANDLE)fp, buffer, len, &i, NULL))
    return 0;
  return i;
#else
  size_t i = fread(buffer, 1, len, (FILE*)fp);
  if(i == 0)
    return 0;
  return (int)i;
#endif
}

int File::write(const char* buffer, int len)
{
#ifdef _WIN32
  DWORD i;
  if(!WriteFile((HANDLE)fp, buffer, len, &i, NULL))
    return 0;
  return i;
#else
  size_t i = fwrite(buffer, 1, len, (FILE*)fp);
  return (int)i;
#endif
}

bool File::write(const String& data)
{
  return write(data.getData(), data.getLength()) == (int)data.getLength();
}

String File::getDirname(const String& file)
{
  const char* start = file.getData();
  const char* pos = &start[file.getLength() - 1];
  for(; pos >= start; --pos)
    if(*pos == '\\' || *pos == '/')
      return file.substr(0, pos - start);
  return String(".");
}

String File::getBasename(const String& file)
{
  const char* start = file.getData();
  const char* pos = &start[file.getLength() - 1];
  for(; pos >= start; --pos)
    if(*pos == '\\' || *pos == '/')
      return file.substr(pos - start + 1);
  return file;
}

String File::getExtension(const String& file)
{
  const char* start = file.getData();
  const char* pos = &start[file.getLength() - 1];
  for(; pos >= start; --pos)
    if(*pos == '.')
      return file.substr(pos - start + 1);
    else if(*pos == '\\' || *pos == '/')
      return String();
  return String();
}

String File::getWithoutExtension(const String& file)
{
  const char* start = file.getData();
  const char* pos = &start[file.getLength() - 1];
  for(; pos >= start; --pos)
    if(*pos == '.')
      return file.substr(0, pos - start);
    else if(*pos == '\\' || *pos == '/')
      return file;
  return file;
}

String File::simplifyPath(const String& path)
{
  String result(path.getLength());
  const char* data = path.getData();
  const char* start = data;
  const char* end;
  String chunck;
  bool startsWithSlash = *data == '/' || *data == '\\';
  for(;;)
  {
    while(*start && (*start == '/' || *start == '\\'))
      ++start;
    end = start;
    while(*end && *end != '/' && *end != '\\')
      ++end;

    if(end == start)
      break;

    chunck = path.substr(start - data, end - start);
    if(chunck == ".." && !result.isEmpty())
    {
      const char* data = result.getData();
      const char* pos = data + result.getLength() - 1;
      for(;; --pos)
        if(pos < data || *pos == '/' || *pos == '\\')
        {
          if(strcmp(pos + 1, "..") != 0)
          {
            if(pos < data)
              result.setLength(0);
            else
              result.setLength(pos - data);
            goto cont;
          }
          break;
        }
    }
    else if(chunck == ".")
      goto cont;

    if(!result.isEmpty() || startsWithSlash)
      result.append('/');
    result.append(chunck);

  cont:
    if(!*end)
      break;
    start = end + 1;
  }
  return result;
}

bool File::isPathAbsolute(const String& path)
{
  const char* data = path.getData();
  return *data == '/' || *data == '\\' || (path.getLength() > 2 && data[1] == ':' && (data[2] == '/' || data[2] == '\\'));
}

bool File::getWriteTime(const String& file, long long& writeTime)
{
#ifdef _WIN32
  WIN32_FIND_DATAA wfd;
  HANDLE hFind = FindFirstFileA(file.getData(), &wfd);
  if(hFind == INVALID_HANDLE_VALUE)
    return false;
  ASSERT(sizeof(DWORD) == 4);
  writeTime = ((long long)wfd.ftLastWriteTime.dwHighDateTime) << 32LL | ((long long)wfd.ftLastWriteTime.dwLowDateTime);
  FindClose(hFind);
  return true;
#else
  struct stat buf;
  if(stat(file.getData(), &buf) != 0)
    return false;
  writeTime = ((long long)buf.st_mtim.tv_sec) * 1000000000LL + ((long long)buf.st_mtim.tv_nsec);
  return true;
#endif
}

bool File::exists(const String& file)
{
#ifdef _WIN32
  WIN32_FIND_DATAA wfd;
  HANDLE hFind = FindFirstFileA(file.getData(), &wfd);
  if(hFind == INVALID_HANDLE_VALUE)
    return false;
  //bool isDir = (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY;
  FindClose(hFind);
  //return isDir;
  return true;
#else
  struct stat buf;
  if(lstat(file.getData(), &buf) != 0)
    return false;
  return true;
#endif
}
