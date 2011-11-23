

#pragma once

#include "Tools/File.h"
#include "Tools/String.h"
#include "Engine.h"
#include "Token.h"

class Statement;

class Parser
{
public:

  Parser(Engine& engine);

  Statement* parse(const String& file, Engine::ErrorHandler errorHandler, void* userData);

private:
  Engine& engine;
  Engine::ErrorHandler errorHandler;
  void* errorHandlerUserData;
  String filePath;
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
  Statement* readFile();

  /** statements ::= '{' { statement } '}' | statement */
  Statement* readStatements();

  /** statement ::= ( 'if' expression statements [ 'else' statements ] | 'input' ( string | quotedstring ) | assignment ) { ( ',' | ';' ) } */
  Statement* readStatement();

  /** assignment ::= '-' ( string | quotedstring ) | ( string | quotedstring ) [ ( '=' | '+=' | '-=' ) expression  */
  Statement* readAssignment();

  /** expression ::= orformula [ '?' expression ':' expression ] */
  Statement* readExpression();

  /** orformula ::= andformula { '||' andformula } */
  Statement* readOrForumla();

  /** andformula ::= comparison { '&&' comparison } */
  Statement* readAndFormula();

  /** comparison ::= relation { ( '==' | '!=' ) relation } */
  Statement* readComparison();

  /** relation ::= concatination { ( '<' | '>' | '<=' | '>=' ) concatination } */
  Statement* readRelation();

  /** concatination ::= value { ( '+' | '-' ) value } */
  Statement* readConcatination();

  /** value ::= '!' value | '(' expression ')' | '{' { statement } '}' | 'true' | 'false' |  string | quotedstring */
  Statement* readValue();
};
