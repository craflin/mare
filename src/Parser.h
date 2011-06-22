

#pragma once

#include "Tools/File.h"
#include "Tools/String.h"
#include "Engine.h"

class Statement;
class Script;

class Parser
{
public:

  Parser(Engine& engine);

  Script* parse(const String& file, Engine::ErrorHandler errorHandler, void* userData);

private:
  class Token
  {
  public:
    enum Id
    {
      leftBrace, rightBrace, comma,
      assignment, plus,
      string, quotedString,
      eof,
      numOfTokens,
    };
    Id id;
    String value;

    const char* getName(Id id) const
    {
      static const char* names[] = {
        "{", "}", ",",
        "=", "+",
        "string", "string",
        "end of file"
      };
      return names[id];
    }
    const char* getName() const {return getName(id);}
  };

  Engine& engine;
  Engine::ErrorHandler errorHandler;
  void* errorHandlerUserData;
  File file;

  char readBuffer[2024];
  const char* readBufferPos;
  const char* readBufferEnd;

  unsigned int currentLine; /**< Starting with 0 */
  char currentChar;
  Token currentToken;

  void nextChar();
  void nextToken();

  void unexpectedChar(char c);
  void expectToken(Token::Id id);

  void readString(String& string);

  /** file = { statement } EOF */
  Script* readFile();

  /** statement ::= assignment { ( ',' | ';' ) } */
  Statement* readStatement();

  /** assignment ::= (string | quotedstring) [ '=' concatination ] */
  Statement* readAssignment();

  /** concatination ::= value { '+' value } */
  Script* readConcatination();

  /** value ::= '{' { statement } '}' | string | quotedstring */
  Statement* readValue();
};
