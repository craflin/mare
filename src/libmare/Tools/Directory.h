/**
* @file Directory.h
* Declaration of a class for accessing directories.
* @author Colin Graf
*/

#pragma once

#include "String.h"
#include "List.h"

/** A Class for accessing directories */
class Directory
{
public:

  /** Default constructor */
  Directory();

  /** Destructor */
  ~Directory();

  /**
  * Opens a directory for searching for files in the directory
  * @param dirpath The path to the directory to search in
  * @param pattern A search pattern like "*.inf"
  * @return Whether the directory was opened successfully
  */
  bool open(const String& dirpath, const String& pattern);

  /**
  * Searches the next matching entry in the opened directory
  * @param dirsOnly 
  * @param path The path of the next matching entry
  * @param isDir Whether the next entry is a directory
  * @return Whether a matching entry was found
  */
  bool read(bool dirsOnly, String& path, bool& isDir);

  static void findFiles(const String& pattern, List<String>& files);

  static bool exists(const String& dir);

  static bool create(const String& dir);

  static bool remove(const String& dir);

private:

#ifdef _WIN32
  void* findFile; /**< Win32 FindFirstFile HANDLE */
  char ffd[320]; /**< Buffer for WIN32_FIND_DATA */
  bool bufferedEntry; /**< Whether there is a buffered search result in ffd. */
  String dirpath; /**< The name of the directory. */
#endif
};
