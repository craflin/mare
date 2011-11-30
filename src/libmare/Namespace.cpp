
#include <cstring>

#include "Tools/Assert.h"
#include "Tools/File.h"
#include "Tools/Directory.h"
#include "Tools/Word.h"
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
      if(cmd == "subst")
      {
        String from, to, text;
        handle(engine, input, from, ",)"); if(*input == ',') ++input;
        handle(engine, input, to, ",)"); if(*input == ',') ++input;
        handle(engine, input, text, ",)"); if(*input == ',') ++input;

        List<Word> words;
        Word::split(text, words);
        for(List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
          i->data.subst(from, to);
        Word::append(words, output);
      }
      else if(cmd == "patsubst")
      {
        String pattern, replace, text;
        handle(engine, input, pattern, ",)"); if(*input == ',') ++input;
        handle(engine, input, replace, ",)"); if(*input == ',') ++input;
        handle(engine, input, text, ",)"); if(*input == ',') ++input;

        List<Word> words;
        Word::split(text, words);
        for(List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
          i->data.patsubst(pattern, replace);
        Word::append(words, output);
      }
      // TODO: strip, findstring
      else if(cmd == "filter" || cmd == "filter-out")
      {
        String pattern, text;
        handle(engine, input, pattern, ",)"); if(*input == ',') ++input;
        handle(engine, input, text, ",)"); if(*input == ',') ++input;

        List<Word> patternwords, words;
        Word::split(pattern, patternwords);
        Word::split(text, words);
        if(cmd == "filter")
          for(List<Word>::Node* i = words.getFirst(), * next; i; i = next)
          {
            next = i->getNext();
            for(List<Word>::Node* j = patternwords.getFirst(); j; j = j->getNext())
              if(i->data.patmatch(j->data))
                goto keepWord;
            words.remove(i);
          keepWord: ;
          }
        else
          for(List<Word>::Node* i = words.getFirst(), * next; i; i = next)
          {
            next = i->getNext();
            for(List<Word>::Node* j = patternwords.getFirst(); j; j = j->getNext())
              if(i->data.patmatch(j->data))
              {
                words.remove(i);
                break;
              }
          }
        Word::append(words, output);
      }
      // TODO: sort, word, wordlist, words
      else if(cmd == "firstword")
      {
        String text;
        handle(engine, input, text, ",)"); if(*input == ',') ++input;

        List<Word> words;
        Word::split(text, words);
        if(!words.isEmpty())
          words.getFirst()->data.appendTo(output);
      }
      else if(cmd == "lastword")
      {
        String text;
        handle(engine, input, text, ",)"); if(*input == ',') ++input;

        List<Word> words;
        Word::split(text, words);
        if(!words.isEmpty())
          words.getLast()->data.appendTo(output);
      }
      else if(cmd == "dir")
      {
        String files;
        handle(engine, input, files, ",)"); if(*input == ',') ++input;

        List<Word> words;
        Word::split(files, words);
        for(List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
          i->data = File::getDirname(i->data);
        Word::append(words, output);
      }
      else if(cmd == "notdir")
      {
        String files;
        handle(engine, input, files, ",)"); if(*input == ',') ++input;
        
        List<Word> words;
        Word::split(files, words);
        for(List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
          i->data = File::getBasename(i->data);
        Word::append(words, output);
      }
      else if(cmd == "suffix")
      {
        String files;
        handle(engine, input, files, ",)"); if(*input == ',') ++input;
        
        List<Word> words;
        Word::split(files, words);
        for(List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
          i->data = File::getExtension(i->data);
        Word::append(words, output);
      }
      else if(cmd == "basename")
      {
        String files;
        handle(engine, input, files, ",)"); if(*input == ',') ++input;
        
        List<Word> words;
        Word::split(files, words);
        for(List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
          i->data = File::getWithoutExtension(i->data);
        Word::append(words, output);
      }
      else if(cmd == "addsuffix")
      {
        String suffix, files;
        handle(engine, input, suffix, ",)"); if(*input == ',') ++input;
        handle(engine, input, files, ",)"); if(*input == ',') ++input;
        
        List<Word> words;
        Word::split(files, words);
        for(List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
          ((String&)i->data).append(suffix);
        Word::append(words, output);
      }
      else if(cmd == "addprefix")
      {
        String prefix, files;
        handle(engine, input, prefix, ",)"); if(*input == ',') ++input;
        handle(engine, input, files, ",)"); if(*input == ',') ++input;
        
        List<Word> words;
        Word::split(files, words);
        for(List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
          i->data.prepend(prefix);
        Word::append(words, output);
      }
      // TODO: wildcard, realpath, abspath
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
      // TODO: or, and
      else if(cmd == "foreach")
      {
        String var, list;
        handle(engine, input, var, ",)"); if(*input == ',') ++input;
        handle(engine, input, list, ",)"); if(*input == ',') ++input;

        List<Word> words;
        Word::split(list, words);
        const char* inputStart = input;
        engine.pushAndLeaveKey();
        engine.enterUnnamedKey();
        engine.enterNewKey(var);
        for(List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
        {
          engine.setKey(i->data);
          engine.pushAndLeaveKey();
          engine.enterUnnamedKey();
          input = inputStart;
          i->data.clear();
          handle(engine, input, i->data, ",)");
          engine.leaveKey(); // unnamed
          engine.popKey();
        }
        engine.leaveKey();
        engine.leaveKey(); // unnamed
        engine.popKey();
        if(*input == ',') ++input;
        Word::append(words, output);
      }
      else if(cmd == "origin")
      {
        String var;
        handle(engine, input, var, ",)"); if(*input == ',') ++input;

        engine.pushAndLeaveKey();
        output.append(engine.getKeyOrigin(var));
        engine.popKey();
      }
      // TODO: call, value, eval, falvor, error, warning, info?
      else if(cmd == "lower")
      {
        String text;
        handle(engine, input, text, ",)"); if(*input == ',') ++input;
        
        List<Word> words;
        Word::split(text, words);
        for(List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
          i->data.lowercase();
        Word::append(words, output);
      }
      else if(cmd == "upper")
      {
        String text;
        handle(engine, input, text, ",)"); if(*input == ',') ++input;
        
        List<Word> words;
        Word::split(text, words);
        for(List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
          i->data.uppercase();
        Word::append(words, output);
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
      engine.pushAndLeaveKey();
      if(engine.enterKey(variable, true))
      {
        engine.appendKeys(output);
        engine.leaveKey();
      }
      engine.popKey();
    }
  };

  String result;
  const char* strData = string.getData();
  ParserAndInterpreter::handle(*engine, strData, result, "");
  return result;
}

Namespace* Namespace::enterKey(const String& name, bool allowInheritance)
{
  compile();

  Word key(name, 0);
  Map<Word, Namespace*>::Node* j = variables.find(key);
  if(j)
  {
    if(!j->data)
      return (j->data = new Namespace(*this, this, engine, 0, 0, 0));
    if(allowInheritance || !(j->data->flags & inheritedFlag))
      return j->data;
  }

  if(allowInheritance)
  {
    Word* word;
    Namespace* space;
    if(engine->resolveScript(name, word, space))
    {
      Namespace* newSpace = space ? new Namespace(*this, this, engine, space->statement, space->next, inheritedFlag) : new Namespace(*this, this, engine, 0, 0, inheritedFlag);
      variables.append(*word, newSpace);
      return newSpace;
    }
  }
  return 0;
}

Namespace* Namespace::enterUnnamedKey(Statement* statement)
{
  return new Namespace(*this, this, engine, statement, 0, unnamedFlag);
}

Namespace* Namespace::enterNewKey(const String& name)
{
  ASSERT(!(flags & compiledFlag));

  Word key(name, 0);
  Map<Word, Namespace*>::Node* j = variables.find(key);
  if(j)
  {
    if(!j->data)
      return (j->data = new Namespace(*this, this, engine, 0, 0, 0));
    if(!(j->data->flags & inheritedFlag))
      return j->data;
  }

  Namespace* space = new Namespace(*this, this, engine, 0, 0, 0);
  variables.append(key, space);
  return space;
}

String Namespace::getKeyOrigin(const String& name)
{
  compile();

  Word key(name, 0);
  Word* word;
  Map<Word, Namespace*>::Node* j = variables.find(key);
  if(j)
    word = &j->key;
  else
  {
    Namespace* space;
    if(!engine->resolveScript(key, word, space))
      return String("undefined");
  }
  if(word->flags & Word::defaultFlag)
    return String("default");
  if(word->flags & Word::commandLineFlag)
    return String("command line");
  return String("file");
}

bool Namespace::resolveScript2(const String& name, Word*& word, Namespace*& result)
{
  ASSERT(!(flags & compilingFlag));
  compile();

  // try a local lookup
  Word key(name, 0);
  Map<Word, Namespace*>::Node* node = variables.find(key);
  if(node)
  {
    result = node->data;
    word = &node->key;
    if(!result)
      return true;
    do
    {
      if(!(result->flags & compilingFlag))
        return true;
      result = result->next;
    } while(result);
  }

  return false;
}

void Namespace::addKey(const String& key, unsigned int wordFlags, Statement* value)
{
  // evaluate variables
  String evaluatedKey = evaluateString(key);

  // textMode?
  if(flags & textModeFlag)
  {
    List<Word> words;
    Word::splitLines(evaluatedKey, words);
    for(const List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
      addKeyRaw(i->data, 0);
    return;
  }

  // split words
  List<Word> words;
  Word::split(evaluatedKey, words);

  // add each word
  for(List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
  {
    Word& word = i->data;
    word.flags |= wordFlags;

    // expand wildcards
    if(word.flags == 0 && strpbrk(word.getData(), "*?")) 
    {
      List<String> files;
      Directory::findFiles(word, files);
      for(const List<String>::Node* i = files.getFirst(); i; i = i->getNext())
        addKeyRaw(Word(i->data, 0), value);
    }
    else
      addKeyRaw(word, value);
  }
}

void Namespace::removeKey(const String& key)
{
  // evaluate variables
  String evaluatedKey = evaluateString(key);

  // textMode?
  if(flags & textModeFlag)
  {
    List<Word> words;
    Word::splitLines(evaluatedKey, words);
    for(const List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
      removeKeyRaw(i->data);
    return;
  }

  // split words
  List<Word> words;
  Word::split(evaluatedKey, words);

  // add each word
  for(const List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
  {
    const Word& word = i->data;

    // expand wildcards
    if(!(word.flags & Word::quotedFlag) && strpbrk(word.getData(), "*?")) 
    {
      List<String> files;
      Directory::findFiles(word, files);
      for(const List<String>::Node* i = files.getFirst(); i; i = i->getNext())
        removeKeyRaw(i->data);
    }
    else
      removeKeyRaw(word);
  }
}

void Namespace::addKeyRaw(const Word& key, Statement* value)
{
  ASSERT(!(flags & compiledFlag));

  // textMode?
  if(flags & textModeFlag)
  {
    variables.append(key, 0);
    return;
  }

  //
  ASSERT(!key.isEmpty());
  Map<Word, Namespace*>::Node* node = variables.find(key);
  if(node)
    node->data = value ? new Namespace(*this, this, engine, value, node->data, 0) : 0;
  else
    variables.append(key, value ? new Namespace(*this, this, engine, value, 0, 0) : 0);
}

void Namespace::removeKeyRaw(const String& key)
{
  ASSERT(!(flags & compiledFlag));
  //ASSERT(!key.isEmpty());
  Word wkey(key, 0);
  Map<Word, Namespace*>::Node* node = variables.find(wkey);
  if(node)
  {
    if(node->data)
      delete node->data;
    variables.remove(node);
  }
}

void Namespace::removeAllKeys()
{
  for(Map<Word, Namespace*>::Node* i = variables.getFirst(); i; i = i->getNext())
    if(i->data)
      delete i->data;
  variables.clear();
}

void Namespace::setKeyRaw(const Word& key)
{
  removeAllKeys();
  variables.append(key, 0);
}

void Namespace::removeKeysRaw(Namespace& space)
{
  space.compile();
  for(const Map<Word, Namespace*>::Node* i = space.variables.getFirst(); i; i = i->getNext())
  {
    if(i->data && (i->data->flags & inheritedFlag))
      break;
    Map<Word, Namespace*>::Node* node = variables.find(i->key);
    if(node)
    {
      if(node->data)
        delete node->data;
      variables.remove(node);
    }
  }
}

bool Namespace::compareKeys(Namespace& space, bool& result)
{
  compile();
  space.compile();

  const Map<Word, Namespace*>::Node* i1 = variables.getFirst();
  const Map<Word, Namespace*>::Node* i2 = space.variables.getFirst();
  for(;;)
  {
    while(i1 && i1->data && (i1->data->flags & inheritedFlag))
      i1 = 0;
    while(i2 && i2->data && (i2->data->flags & inheritedFlag))
      i2 = 0;
    if(!i1 && !i2)
    {
      result = true;
      return true;
    }
    if(!i1 || !i2 || i1->key != i2->key)
    {
      result = false;
      return true;
    }
    i1 = i1->getNext();
    i2 = i2->getNext();
  }
  ASSERT(false);
  return false;
}

#include <cstdlib>
bool Namespace::versionCompareKeys(Namespace& space, int& result)
{
  compile();
  space.compile();

  // TODO: compare versions not numbers
  result = atoi(getFirstKey().getData()) - atoi(space.getFirstKey().getData());

  return true;
}

void Namespace::addDefaultStatement(Statement* statement)
{
  ASSERT(!(flags & compiledFlag));
  if(!defaultStatement)
    defaultStatement = statement;
  else
  {
    BlockStatement* blockStatement = dynamic_cast<BlockStatement*>(defaultStatement);
    if(blockStatement)
      blockStatement->statements.append(statement);
    else
    {
      blockStatement = new BlockStatement(*this);
      blockStatement->statements.append(defaultStatement);
      blockStatement->statements.append(statement);
      defaultStatement = blockStatement;
    }
  }
}

void Namespace::addDefaultKey(const String& key)
{
  StringStatement* stringStatement = new StringStatement(*this);
  stringStatement->value = key;
  addDefaultStatement(stringStatement);
}

void Namespace::addDefaultKey(const String& key, unsigned int flags, const String& value)
{
  StringStatement* stringStatement = new StringStatement(*this);
  stringStatement->value = value;
  AssignStatement* assignStatement = new AssignStatement(*this);
  assignStatement->variable = key;
  assignStatement->flags = flags;
  assignStatement->value = stringStatement;
  addDefaultStatement(assignStatement);
}

void Namespace::addDefaultKey(const String& key, unsigned int flags, const Map<String, String>& value)
{
  BlockStatement* blockStatement = new BlockStatement(*this);
  for(const Map<String, String>::Node* i = value.getFirst(); i; i = i->getNext())
  {
    StringStatement* stringStatement = new StringStatement(*this);
    stringStatement->value = i->data;
    AssignStatement* assignStatement = new AssignStatement(*this);
    assignStatement->variable = i->key;
    assignStatement->value = stringStatement;
    blockStatement->statements.append(assignStatement);
  }
  AssignStatement* assignStatement = new AssignStatement(*this);
  assignStatement->variable = key;
  assignStatement->flags = flags;
  assignStatement->value = blockStatement;
  addDefaultStatement(assignStatement);
}

void Namespace::getKeys(List<String>& keys)
{
  compile();
  for(Map<Word, Namespace*>::Node* node = variables.getFirst(); node; node = node->getNext())
  {
    if(node->data && (node->data->flags & inheritedFlag))
      break;
    keys.append(node->key);
  }
}

void Namespace::getText(List<String>& text)
{
  // recompile in textMode
  ASSERT(!(flags & compilingFlag));
  removeAllKeys();
  flags &= ~compiledFlag;
  flags |= textModeFlag;
  compile();
  flags &= ~(compiledFlag | textModeFlag);

  // copy each text line
  for(Map<Word, Namespace*>::Node* i = variables.getFirst(); i; i = i->getNext())
    text.append(i->key);
}

void Namespace::appendKeys(String& output)
{
  compile();
  for(Map<Word, Namespace*>::Node* i = variables.getFirst(); i; i = i->getNext())
  {
    if(i->data && (i->data->flags & inheritedFlag))
      break;
    if(i != variables.getFirst())
      output.append(' ');
    i->key.appendTo(output);
  }
}

String Namespace::getFirstKey()
{
  compile();
  for(Map<Word, Namespace*>::Node* node = variables.getFirst(); node; node = node->getNext())
  {
    if(node->data && (node->data->flags & inheritedFlag))
      break;
    return node->key;
  }
  return String();
}

void Namespace::compile()
{
  if(flags & compiledFlag)
    return;
  if(flags & compilingFlag)
  {
    //ASSERT(false);
    return;
  }
  flags |= compilingFlag;
  if(defaultStatement)
    defaultStatement->execute(*this);
  if(statement)
    statement->execute(*this);
  flags &= ~compilingFlag;
  flags |= compiledFlag;
}
