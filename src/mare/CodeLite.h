
#pragma once

#include "Generator.h"

class CodeLite : public Generator
{
public:
  CodeLite(Engine& engine) : Generator(engine) {}

private:
  class ProjectFile;

  class FileTreeNode
  {
  public:
    Map<String, FileTreeNode*> folders;
    Map<String, const ProjectFile*> files;

    ~FileTreeNode()
    {
      for(Map<String, FileTreeNode*>::Node* i = folders.getFirst(); i; i = i->getNext())
        delete i->data;
    }
  };

  class ProjectConfiguration
  {
  public:
    const Target* target;
  };

  class ProjectFile
  {
  public:
    class Configuration
    {
    public:
      const File* file;
    };

    Map<String, Configuration> configurations;
    String folder;
  };

  class Project
  {
  public:
    Map<String, ProjectConfiguration> configurations;
    Map<String, ProjectFile> files;
    Map<String, void*> roots;
    FileTreeNode fileTree;
  };

  String workspaceName;
  List<String> configurations;
  Map<String, Project> projects;


  virtual void addDefaultKeys(Engine& engine);
  virtual bool processData(const Data& data);
  virtual bool writeFiles();

  bool writeWorkspace();
  bool writeProject(const String& projectName, const Project& project);

  static String join(const List<String>& items, char sep = ';', const String& suffix = String());
  static String joinCommands(const List<String>& commands);
  static String xmlEscape(const String& text);
};
