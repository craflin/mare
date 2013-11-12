/**
* @file Directory.cpp
* Implementation of a class for accessing directories.
* @author Colin Graf
*/

#include <cstring>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>
#include <cerrno>
#endif

#include "Assert.h"
#include "Directory.h"
#include "List.h"
#include "Map.h"
#include "File.h"

Directory::Directory()
{
#ifdef _WIN32
  findFile = INVALID_HANDLE_VALUE;
  ASSERT(sizeof(ffd) >= sizeof(WIN32_FIND_DATA));
#else
  dp = 0;
#endif
}

Directory::~Directory()
{
#ifdef _WIN32
  if(findFile != INVALID_HANDLE_VALUE)
    FindClose((HANDLE)findFile);
#else
  if(dp)
    closedir((DIR*)dp);
#endif
}

bool Directory::remove(const String& dir)
{
  String path = dir;
  while(path != ".")
  {
#ifdef _WIN32
    if(!RemoveDirectory(path.getData()))
      return false;
#else
    if(rmdir(path.getData()) != 0)
      return false;
#endif
    path = File::getDirname(path);
  }
  return true;
}

bool Directory::open(const String& dirpath, const String& pattern, bool dirsOnly)
{
#ifdef _WIN32
  if(findFile != INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return false;
  }

  this->dirsOnly = dirsOnly;
  this->dirpath = dirpath;
  const char* patExt = strrchr(pattern.getData(), '.');
  this->patternExtension = (patExt && !strpbrk(patExt + 1, "*?")) ? String(patExt + 1, -1) : String();

  String searchPath = dirpath;
  searchPath.setCapacity(dirpath.getLength() + 1 + pattern.getLength());
  if(!dirpath.isEmpty())
    searchPath.append('/');
  searchPath.append(pattern);

  findFile = FindFirstFileEx(searchPath.getData(),
#if _WIN32_WINNT > 0x0600
	  FindExInfoBasic,
#else
	  FindExInfoStandard,
#endif
	  (LPWIN32_FIND_DATA)ffd, dirsOnly ? FindExSearchLimitToDirectories : FindExSearchNameMatch, NULL, 0);
  if(findFile == INVALID_HANDLE_VALUE)
    return false;
  bufferedEntry = true;
  return true;
#else
  if(dp)
  {
    errno = EINVAL;
    return false;
  }

  this->dirsOnly = dirsOnly;
  this->dirpath = dirpath;
  this->pattern = pattern;

  dp = opendir(dirpath.getData());
  return dp != 0;
#endif
}

bool Directory::read(String& name, bool& isDir)
{
#ifdef _WIN32
  if(!findFile)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return false;
  }
  for(;;)
  {
    if(bufferedEntry)
      bufferedEntry = false;
    else if(!FindNextFile((HANDLE)findFile, (LPWIN32_FIND_DATA)ffd))
    {
      DWORD lastError = GetLastError();
      FindClose((HANDLE)findFile);
      findFile = INVALID_HANDLE_VALUE;
      SetLastError(lastError);
      return false;
    }
    isDir = (((LPWIN32_FIND_DATA)ffd)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
    if(dirsOnly && !isDir)
      continue;
    const char* str = ((LPWIN32_FIND_DATA)ffd)->cFileName;
    if(isDir && *str == '.' && (str[1] == '\0' || (str[1] == '.' && str[2] == '\0')))
      continue;

    if(!patternExtension.isEmpty())
    {
      const char* patExt = strrchr(str, '.');
      if(!patExt || _stricmp(patternExtension.getData(), patExt + 1) != 0)
        continue;
    }

    name = String(str, -1);
    return true;
  }
#else
  if(!dp)
  {
    errno = EINVAL;
    return false;
  }

  for(;;)
  {
    struct dirent* dent = readdir((DIR*)dp);
    if(!dent)
    {
      int lastErrno = errno;
      closedir((DIR*)dp);
      dp = 0;
      errno = lastErrno;
      return false;
    }
    const char* const str = dent->d_name;
    if(fnmatch(pattern.getData(), str, 0) == 0)
    {
      name = String(str, -1);
      isDir = false;
      if(dent->d_type == DT_DIR)
        isDir = true;
      else if(dent->d_type == DT_LNK || dent->d_type == DT_UNKNOWN)
      {
        String path = dirpath;
        path.append('/');
        path.append(name);
        struct stat buff;
        if(stat(path.getData(), &buff) == 0)
          if(S_ISDIR(buff.st_mode))
            isDir = true;
      }
      if(dirsOnly && !isDir)
        continue;
      if(isDir && *str == '.' && (str[1] == '\0' || (str[1] == '.' && str[2] == '\0')))
        continue;
      return true;
    }
  }
  return false; // unreachable
#endif
}

void Directory::findFiles(const String& pattern, List<String>& files)
{
  // replace ** with * / ** and split in chunks
  List<String> chunks;
  int start = 0, i = 0;
  for(const char* str = pattern.getData(), * end = str + pattern.getLength(); str <= end; ++str, ++i)
    switch(*str)
    {
    case '/':
    case '\\':
    case '\0':
      int len = i - start;
      if(len > 0)
        chunks.append(pattern.substr(start, len));
      start = i + 1;
      break;
    }

  //
  struct FindFiles
  {
    List<String>* files;
    bool dirsOnly;

    void handlePath(const String& path, const String& pattern, const List<String>::Node* nextChunk)
    {
      const char* useWildcards = strpbrk(pattern.getData(), "*?");
      const char* doubleStar = useWildcards ? strstr(useWildcards, "**") : 0;
      if(doubleStar)
      {
        /*
          if a**b / c
            use a*b / c
            use a* / **b / c
          if a** / c
            use a* / c   //?
            use a* / ** / c
          if **b / c
            use *b / c
            use * / **b / c
          if ** /
            use *
            use * / **
          if ** / c
            use * / c //?
            //use c //?
            use * / ** / c
        */

        const List<String>::Node* nextNextChunk = 0;
        String nextPattern;
        if(nextChunk)
        {
          nextNextChunk = nextChunk->getNext();
          nextPattern = nextChunk->data;
        }

        if(doubleStar != pattern.getData())
        {
          if (doubleStar != pattern.getData() + pattern.getLength() - 2) // pattern == a**b
          {
            String subPattern(pattern.getLength());
            subPattern.append(pattern.getData(), doubleStar - pattern.getData()); // = a
            subPattern.append(doubleStar + 1, pattern.getLength() - (doubleStar + 1 - pattern.getData())); // += *b
            handlePath2(path, subPattern, nextPattern, nextNextChunk);
            subPattern.clear();
            subPattern.append(pattern.getData(), doubleStar + 1 - pattern.getData());
            String nextPattern2(doubleStar, pattern.getLength() - (doubleStar - pattern.getData()));
            handlePath2(path, subPattern, nextPattern2, nextChunk);
          }
          else // pattern == a**
          {
            String subPattern(pattern.getData(), pattern.getLength() - 1);
            handlePath2(path, subPattern, nextPattern, nextNextChunk);
            handlePath2(path, subPattern, "**", nextChunk);
          }
        }
        else // pattern == **b || pattern == **
        {
          if(pattern.getLength() != 2) // pattern == **b
          {
            handlePath2(path, String(doubleStar + 1, pattern.getLength() - 1), nextPattern, nextNextChunk);
            handlePath2(path, "*", pattern, nextChunk);
          }
          else // pattern == **
          {
            handlePath2(path, "*", nextPattern, nextNextChunk);
            handlePath2(path, "*", pattern, nextChunk);
          }
        }
      }
      else if(useWildcards || nextChunk == 0)
      {
        const List<String>::Node* nextNextChunk = 0;
        String nextPattern;
        if(nextChunk)
        {
          nextNextChunk = nextChunk->getNext();
          nextPattern = nextChunk->data;
        }
        handlePath2(path, pattern, nextPattern, nextNextChunk);
      }
      else // not a pattern
        handleSubPath(path, pattern, true, nextChunk->data, nextChunk->getNext());
    }

    void handlePath2(const String& path, const String& pattern, const String& nextPattern, const List<String>::Node* nextNextChunk)
    {
      Directory dir; String name; bool isDir;
      if(dir.open(path, pattern, !nextPattern.isEmpty() || dirsOnly))
        while(dir.read(name, isDir))
          handleSubPath(path, name, isDir, nextPattern, nextNextChunk);
    }

    void handleSubPath(const String& path, const String& name, bool isDir, const String& nextPattern, const List<String>::Node* nextNextChunk)
    {
      String subpath = path;
      subpath.setCapacity(path.getLength() + 2 + name.getLength());
      if(!path.isEmpty())
        subpath.append('/');
      subpath.append(name);
      if(nextPattern.isEmpty())
      {
        if(dirsOnly)
        {
          if(isDir)
          {
            subpath.append('/');
            files->append(subpath);
          }
        }
        else
          files->append(subpath);
      }
      else if(isDir)
        handlePath(subpath, nextPattern, nextNextChunk);
    }
  };

  if(!chunks.isEmpty())
  {
    FindFiles ff;
    char lastChar = pattern.getData()[pattern.getLength() - 1];
    ff.dirsOnly = lastChar == '/' || lastChar == '\\';
    ff.files = &files;
    ff.handlePath(String(), chunks.getFirst()->data, chunks.getFirst()->getNext());
  }
}

bool Directory::exists(const String& dir)
{
#ifdef _WIN32
  WIN32_FIND_DATAA wfd;
  HANDLE hFind = FindFirstFileA(dir.getData(), &wfd);
  if(hFind == INVALID_HANDLE_VALUE)
    return false;
  bool isDir = (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
  FindClose(hFind);
  return isDir;
#else
  struct stat buf;
  if(stat(dir.getData(), &buf) != 0)
    return false;
  return S_ISDIR(buf.st_mode);
#endif
}

bool Directory::create(const String& dir)
{
  // TODO: set errno correctly

  static Map<String, bool> createdDirs;
  Map<String, bool>::Node* i  = createdDirs.find(dir);
  if(i)
    return i->data;

  if(exists(dir))
  {
    createdDirs.append(dir, true);
    return true;
  }


  const char* start = dir.getData();
  const char* pos = &start[dir.getLength() - 1];
  for(; pos >= start; --pos)
    if(*pos == '\\' || *pos == '/')
    {
      if(!create(dir.substr(0, pos - start)))
      {
        createdDirs.append(dir, false);
        return false;
      }
      break;
    }
  ++pos;
  bool result = false;
  if(*pos)
#ifdef _WIN32
    result = CreateDirectory(dir.getData(), NULL) == TRUE;
#else
    result = mkdir(dir.getData(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0;
#endif
  createdDirs.append(dir, result);
  return result;
}

bool Directory::change(const String& dir)
{
#ifdef _WIN32
  return SetCurrentDirectory(dir.getData()) != FALSE;
#else
  return chdir(dir.getData()) == 0;
#endif
}

String Directory::getCurrent()
{
#ifdef _WIN32
  TCHAR buffer[MAX_PATH];
  DWORD len = GetCurrentDirectory(MAX_PATH, buffer);
  return String(buffer, len);
#else
  char buffer[PATH_MAX];
  if(!getcwd(buffer, PATH_MAX))
    return String();
  return String(buffer, strlen(buffer));
#endif
}
