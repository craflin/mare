
#include <cstring>
#include <cassert>

#include "Tools/File.h"
#include "Tools/Directory.h"
#include "Tools/Words.h"
#include "Namespace.h"
#include "Statement.h"
#include "Engine.h"
#include "Script.h"

String Namespace::evaluateString(const String& string)
{
  /*
  string = { chunk }
  chunk = '$(' vardecl ')' | chars
  vardecl = string [ ' ' string { ',' string } ]
  */

  struct ParserAndInterpreter
  {
    static void handle(Engine& engine, const char*& input, String& output, const char* endchars)
    {
      while(*input && !strchr(endchars, *input))
        if(*input == '$' && input[1] == '(')
        {
          input += 2;
          String varOrCommand;
          handle(engine, input, varOrCommand, " )");
          if(*input == ' ')
          {
            ++input;
            handleCommand(engine, varOrCommand, input, output);
            if(*input && *input != ')')
              for(;;)
              {
                handle(engine, input, varOrCommand, ",)");
                if(*input != ',')
                  break;
                ++input;
              }
          }
          else
            handleVariable(engine, varOrCommand, output);
          if(*input == ')')
            ++input;
        }
        else
        {
          const char* str = input++;
          while(*input && *input != '$' && !strchr(endchars, *input))
            ++input;
          output.append(str, input - str);
        }
    }
    static void handleCommand(Engine& engine, const String& cmd, const char*& input, String& output)
    {
      if(cmd == "patsubst")
      {
        String pattern, replace, text;
        handle(engine, input, pattern, ",)"); if(*input == ',') ++input;
        handle(engine, input, replace, ",)"); if(*input == ',') ++input;
        handle(engine, input, text, ",)"); if(*input == ',') ++input;

        List<String> words;
        Words::split(text, words);
        for(List<String>::Node* i = words.getFirst(); i; i = i->getNext())
          i->data.patsubst(pattern, replace);
        Words::append(words, output);
      }
      else if(cmd == "subst")
      {
        String from, to, text;
        handle(engine, input, from, ",)"); if(*input == ',') ++input;
        handle(engine, input, to, ",)"); if(*input == ',') ++input;
        handle(engine, input, text, ",)"); if(*input == ',') ++input;

        List<String> words;
        Words::split(text, words);
        for(List<String>::Node* i = words.getFirst(); i; i = i->getNext())
          i->data.subst(from, to);
        Words::append(words, output);
      }
      else if(cmd == "firstword")
      {
        String text;
        handle(engine, input, text, ",)"); if(*input == ',') ++input;

        List<String> words;
        Words::split(text, words);
        if(!words.isEmpty())
          output.append(words.getFirst()->data);
      }
      else if(cmd == "foreach")
      {
        String var, list;
        handle(engine, input, var, ",)"); if(*input == ',') ++input;
        handle(engine, input, list, ",)"); if(*input == ',') ++input;

        List<String> words;
        Words::split(list, words);
        const char* inputStart = input;
        engine.enterUnnamedKey();
        for(List<String>::Node* i = words.getFirst(); i; i = i->getNext())
        {
          engine.addDefaultKey(var, i->data);
          engine.enterUnnamedKey();
          input = inputStart;
          i->data.clear();
          handle(engine, input, i->data, ",)");
          engine.leaveKey();
        }
        engine.leaveKey();
        if(*input == ',') ++input;
        Words::append(words, output);
      }
      else if(cmd == "filter" || cmd == "filter-out")
      {
        String pattern, text;
        handle(engine, input, pattern, ",)"); if(*input == ',') ++input;
        handle(engine, input, text, ",)"); if(*input == ',') ++input;

        List<String> patternwords, words;
        Words::split(pattern, patternwords);
        Words::split(text, words);
        if(cmd == "filter")
          for(List<String>::Node* i = words.getFirst(), * next; i; i = next)
          {
            next = i->getNext();
            for(List<String>::Node* j = patternwords.getFirst(); j; j = j->getNext())
              if(i->data.patmatch(j->data))
                goto keepWord;
            words.remove(i);
          keepWord: ;
          }
        else
          for(List<String>::Node* i = words.getFirst(), * next; i; i = next)
          {
            next = i->getNext();
            for(List<String>::Node* j = patternwords.getFirst(); j; j = j->getNext())
              if(i->data.patmatch(j->data))
              {
                words.remove(i);
                break;
              }
          }
        Words::append(words, output);
      }
      else if(cmd == "readfile")
      {
        String filepath;
        handle(engine, input, filepath, ",)"); if(*input == ',') ++input;

        File file;
        if(file.open(filepath))
        {
          char buffer[2048];
          int i;
          while((i = file.read(buffer, sizeof(buffer))) > 0)
            output.append(buffer, i);
        }
      }
    }
    static void handleVariable(Engine& engine, const String& variable, String& output)
    {
      engine.pushState();
      engine.leaveKey();
      List<String> keys;
      engine.getKeys(variable, keys, true);
      engine.popState();
      Words::append(keys, output);
    }
  };

  String result;
  const char* strData = string.getData();
  ParserAndInterpreter::handle(*engine, strData, result, "");
  return result;
}

Namespace* Namespace::enterKey(const String& name, bool allowInheritance)
{
  if(compile())
  {
    Map<String, Variable>::Node* i = variables.find(name);
    if(i)
    {
      if(!i->data.space)
        i->data.space = new Namespace(*this, this, engine, i->data.script, name);
      return i->data.space;
    }
  }

//failure:
  if(allowInheritance)
  {
    Script* script;
    if(engine->resolveScript(name, script))
    {
      Variable& var = variables.append(name, Variable(script, true));
      var.space = new Namespace(*this, this, engine, script, name);
      return var.space;
    }
  }
  return 0;
}

Namespace* Namespace::enterUnnamedKey()
{
  if(unnamedSpace)
    delete unnamedSpace;
  unnamedSpace = new Namespace(*this, this, engine, 0, "<unnamed>");
  return unnamedSpace;
}

Namespace* Namespace::enterDefaultKey(const String& name)
{
  Map<String, Variable>::Node* i = variables.find(name);
  if(!i)
  {
    Variable& var = variables.append(name, 0);
    var.space = new Namespace(*this, this, engine, 0, name);
    return var.space;
  }
  if(!i->data.space)
    i->data.space = new Namespace(*this, this, engine, i->data.script, name);
  return i->data.space;
}

bool Namespace::resolveScript(const String& name, Script*& script)
{
  if(!compile())
    return false;
  Map<String, Variable>::Node* i = variables.find(name);
  if(!i)
    return false;
  script = i->data.script;
  return true;
}

void Namespace::addKey(const String& key, Script* value)
{
  // evaluate variables
  String evaluatedKey = evaluateString(key);

  // split words
  List<String> words;
  Words::split(evaluatedKey, words);

  // add each word
  for(const List<String>::Node* i = words.getFirst(); i; i = i->getNext())
  {
    const String& word = i->data;

    // expand wildcards
    if(strpbrk(word.getData(), "*?")) 
    {
      List<String> files;
      Directory::findFiles(word, files);
      for(const List<String>::Node* i = files.getFirst(); i; i = i->getNext())
        addKeyRaw(i->data, value);
    }
    else
      addKeyRaw(word, value);
  }
}

void Namespace::addKeyRaw(const String& key, Script* value)
{
  assert(!compiled);
  assert(!key.isEmpty());
  Map<String, Variable>::Node* node = variables.find(key);
  if(node)
  {
    node->data.script = value;
    if(node->data.space)
    {
      delete node->data.space;
      node->data.space = 0;
    }
  }
  else
    variables.append(key, value);
}

void Namespace::addKeyRaw(const String& key, const String& value)
{
  //assert(!compiled);
  assert(!key.isEmpty());
  Map<String, Variable>::Node* node = variables.find(key);
  if(node)
  {
    StringScript* script = dynamic_cast<StringScript*>(node->data.script);
    if(script)
    {
      if(node->data.space)
        node->data.space->reset();
      script->value = value;
    }
    else
    {
      node->data = new StringScript(*this, value);
      if(node->data.space)
      {
        delete node->data.space;
        node->data.space = 0;
      }
    }
  }
  else
    variables.append(key, new StringScript(*this, value));
}

void Namespace::getKeys(List<String>& keys)
{
  if(!compile())
    return;
  for(Map<String, Variable>::Node* node = variables.getFirst(); node; node = node->getNext())
    if(!node->data.inherited)
      keys.append(node->key);
}

String Namespace::getFirstKey()
{
  if(!compile())
    return String();
  for(Map<String, Variable>::Node* node = variables.getFirst(); node; node = node->getNext())
    if(!node->data.inherited)
      return node->key;
  return String();
}

//#include <Windows.h>
bool Namespace::compile()
{
  if(compiled)
    return true;

  /*
  if(compiling)
    return false;

  String debugStr = name;
  for(Namespace* i = parent; i; i = i->parent)
  {
    String str = i->name;
    str.append('.');
    str.append(debugStr);
    debugStr = str;
  }
  debugStr.append("\n");
  OutputDebugString(debugStr.getData());

  compiling = true;
  */
  if(script)
    if(!script->execute(*this))
      return false;
  //compiling = false;
  compiled = true;
  return true;
}

void Namespace::reset()
{
  compiled = false;
  variables.clear();
}
