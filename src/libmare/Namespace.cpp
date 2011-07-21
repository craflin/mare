
#include <cstring>
#include <cassert>

#include "Tools/File.h"
#include "Tools/Directory.h"
#include "Tools/Words.h"
#include "Namespace.h"
#include "Statement.h"
#include "Engine.h"

String Namespace::evaluateString(const String& string)
{
  /*
  string = { chunk }
  chunk = '$(' vardecl ')' | chars
  vardecl = string [ ' ' string { ',' string } ]
  */

  struct ParserAndInterpreter
  {
    static void handle(Engine& engine, const char*& input, String& output, const char* endchars, bool evaluate = true)
    {
      while(*input && !strchr(endchars, *input))
        if(*input == '$' && input[1] == '(')
        {
          input += 2;
          String varOrCommand;
          handle(engine, input, varOrCommand, " )", evaluate);
          if(*input == ' ')
          {
            ++input;
            if(evaluate)
              handleCommand(engine, varOrCommand, input, output);

            // skip further arguements
            if(*input && *input != ')')
              for(;;)
              {
                handle(engine, input, output, ",)", false);
                if(*input != ',')
                  break;
                ++input;
              }
          }
          else
          {
            if(evaluate)
              handleVariable(engine, varOrCommand, output);
          }
          if(*input == ')')
            ++input;
        }
        else
        {
          const char* str = input++;
          while(*input && *input != '$' && !strchr(endchars, *input))
            ++input;
          if(evaluate)
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
        engine.enterDefaultKey(var);
        for(List<String>::Node* i = words.getFirst(); i; i = i->getNext())
        {
          engine.pushKey();
          engine.setDefaultKey(i->data);
          engine.leaveKey();
          engine.enterUnnamedKey();
          input = inputStart;
          i->data.clear();
          handle(engine, input, i->data, ",)");
          engine.popKey();
        }
        engine.leaveKey();
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
      else if(cmd == "if")
      {
        String condition;
        handle(engine, input, condition, ",)"); if(*input == ',') ++input;
        if(!condition.isEmpty())
        { // then
          handle(engine, input, output, ",)"); if(*input == ',') ++input;
          handle(engine, input, output, ",)", false); if(*input == ',') ++input;
        }
        else
        {
          handle(engine, input, output, ",)", false); if(*input == ',') ++input;
          handle(engine, input, output, ",)"); if(*input == ',') ++input;
        }
      }
    }
    static void handleVariable(Engine& engine, const String& variable, String& output)
    {
      engine.pushKey();
      engine.leaveKey();
      List<String> keys;
      engine.getKeys(variable, keys, true);
      engine.popKey();
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
  if(!compile())
    goto failure;

  {
    Map<String, Namespace*>::Node* j = variables.find(name);
    if(j && (allowInheritance || !j->data->inherited))
      return j->data;
  }

failure:
  if(allowInheritance)
  {
    Namespace* space;
    if(engine->resolveScript(name, space))
    {
      Namespace* newSpace = new Namespace(*this, this, engine, space->statement, space->next, true);
      variables.append(name, newSpace);
      return newSpace;
    }
  }
  return 0;
}

Namespace* Namespace::enterUnnamedKey()
{
  if(unnamedSpace)
    delete unnamedSpace;
  unnamedSpace = new Namespace(*this, this, engine, 0, 0, false);
  return unnamedSpace;
}

Namespace* Namespace::enterDefaultKey(const String& name)
{
  assert(!compiled);

  Map<String, Namespace*>::Node* j = variables.find(name);
  if(j && !j->data->inherited)
    return j->data;

  Namespace* space = new Namespace(*this, this, engine, 0, 0, false);
  variables.append(name, space);
  return space;
}

bool Namespace::resolveScript(const String& name, Namespace*& space)
{
  if(!compile())
    return false;

  Map<String, Namespace*>::Node* j = variables.find(name);
  if(j)
    space = j->data;
  else
    return false;

  while(space->compiling)
  {
    space = space->next;
    if(!space)
      return false;
  }

  return true;
}

void Namespace::addKey(const String& key, Statement* value)
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

void Namespace::addKeyRaw(const String& key, Statement* value)
{
  assert(!compiled);
  assert(!key.isEmpty());
  Map<String, Namespace*>::Node* node = variables.find(key);
  if(node)
    node->data = new Namespace(*this, this, engine, value, node->data, false);
  else
    variables.append(key, new Namespace(*this, this, engine, value, 0, false));
}

void Namespace::setKeyRaw(const String& key)
{
  variables.clear();
  variables.append(key, 0);
}

void Namespace::removeKeys(Namespace& space)
{
  if(!space.compile())
    return;
  for(const Map<String, Namespace*>::Node* i = space.variables.getFirst(); i; i = i->getNext())
  {
    Map<String, Namespace*>::Node* node = variables.find(i->key);
    if(node)
      variables.remove(node);
  }
}

bool Namespace::compareKeys(Namespace& space, bool& result)
{
  if(!compile() || !space.compile())
    return false;

  result = true;
  if(variables.getSize() != space.variables.getSize())
    result = false;
  else
    for(const Map<String, Namespace*>::Node* i1 = variables.getFirst(), * i2 = space.variables.getFirst(); i2; i1 = i1->getNext(), i2 = i2->getNext())
      if(i1->key != i2->key)
      {
        result = false;
        break;
      }

  return true;
}

#include <cstdlib>
bool Namespace::versionCompareKeys(Namespace& space, int& result)
{
  if(!compile() || !space.compile())
    return false;

  // TODO: compare versions not numbers
  result = atoi(getFirstKey().getData()) - atoi(space.getFirstKey().getData());

  return true;
}

void Namespace::addResolvableKey(const String& key, const String& value)
{
  assert(!compiled);
  Statement* newStatement;
  if(value.isEmpty())
  {
    StringStatement* stringStatement = new StringStatement(*this);
    stringStatement->value = key;
    newStatement = stringStatement;
  }
  else
  {
    StringStatement* stringStatement = new StringStatement(*this);
    stringStatement->value = value;
    AssignStatement* assignStatement = new AssignStatement(*this);
    assignStatement->variable = key;
    assignStatement->value = stringStatement;
    newStatement = assignStatement;
  }
  if(!statement)
    statement = newStatement;
  else
  {
    BlockStatement* blockStatement = dynamic_cast<BlockStatement*>(statement);
    if(blockStatement)
      blockStatement->statements.append(newStatement);
    else
    {
      blockStatement = new BlockStatement(*this);
      blockStatement->statements.append(statement);
      blockStatement->statements.append(newStatement);
      statement = blockStatement;
    }
  }
}

void Namespace::getKeys(List<String>& keys)
{
  if(!compile())
    return;
  for(Map<String, Namespace*>::Node* node = variables.getFirst(); node; node = node->getNext())
    if(!node->data || !node->data->inherited)
      keys.append(node->key);
}

String Namespace::getFirstKey()
{
  if(!compile())
    return String();
  for(Map<String, Namespace*>::Node* node = variables.getFirst(); node; node = node->getNext())
    if(!node->data || !node->data->inherited)
      return node->key;
  return String();
}

bool Namespace::compile()
{
  if(compiled)
    return true;
  if(compiling)
    return false;
  compiling = true;
  if(statement)
    statement->execute(*this);
  compiling = false;
  compiled = true;
  return true;
}
/*
void Namespace::reset()
{
  compiled = false;
  variables.clear();
  spaces.clear();
}
*/
