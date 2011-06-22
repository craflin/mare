
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
        engine.enterNew();
        for(List<String>::Node* i = words.getFirst(); i; i = i->getNext())
        {
          engine.addDefaultVariable(var, i->data);
          input = inputStart;
          i->data.clear();
          handle(engine, input, i->data, ",)");
        }
        engine.leave();
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

      List<String> keys;
      engine.getKeys(variable, keys, true);
      Words::append(keys, output);
    }
  };

  String result;
  const char* strData = string.getData();
  ParserAndInterpreter::handle(*engine, strData, result, "");
  return result;
}

void Namespace::addVariable(const String& key, Script* value)
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
        addVariableRaw(i->data, value);
    }
    else
      addVariableRaw(word, value);
  }
  
  /*
  // expand wildcards
  if(strpbrk(key.getData(), "*?")) 
  {
    List<String> files;
    Directory::findFiles(key, files);
    for(const List<String>::Node* i = files.getFirst(); i; i = i->getNext())
      addVariableRaw(i->data, value);
  }
  else
    addVariableRaw(key, value);
    */
}

void Namespace::addVariableRaw(const String& key, Script* value)
{
  assert(!key.isEmpty());
  Map<String, Script*>::Node* node = variables.find(key);
  if(node)
  {
    if(node->data)
      delete node->data;
    node->data = value;
  }
  else
    variables.append(key, value);
}

void Namespace::addDefaultVariableRaw(const String& key, const String& value)
{
  assert(!key.isEmpty());
  Map<String, Script*>::Node* node = variables.find(key);
  if(node)
  {
    StringScript* script = dynamic_cast<StringScript*>(node->data);
    if(script)
      script->value = value;
    else
    {
      if(node->data)
        delete node->data;
      node->data = new StringScript(*this, value);
    }
  }
  else
    variables.append(key, new StringScript(*this, value));
}

bool Namespace::resolve(const String& name, Script*& script)
{
  compile();
  Map<String, Script*>::Node* i = variables.find(name);
  if(!i)
    return false;
  script = i->data;
  return true;
}

void Namespace::getKeys(List<String>& keys)
{
  compile();
  keys.clear();
  for(Map<String, Script*>::Node* node = variables.getFirst(); node; node = node->getNext())
    keys.append(node->key);
}

String Namespace::getFirstKey()
{
  compile();
  Map<String, Script*>::Node* node = variables.getFirst();
  if(node)
    return node->key;
  return String();
}

void Namespace::compile()
{
  if(compiled)
    return;
  if(script)
    script->execute(*this);
  compiled = true;
}
