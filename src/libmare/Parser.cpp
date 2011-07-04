
#include <cctype>
#include <cstring>

#include "Parser.h"
#include "Tools/String.h"
#include "Statement.h"
#include "Script.h"

Parser::Parser(Engine& engine) : engine(engine), readBufferPos(0), readBufferEnd(0), currentLine(1) {}

Script* Parser::parse(const String& file, Engine::ErrorHandler errorHandler, void* userData)
{
  this->errorHandler = errorHandler;
  this->errorHandlerUserData = userData;

  try
  {
    if(!this->file.open(file))
    {
      errorHandler(errorHandlerUserData, 0, this->file.getErrno().getString());
      throw false;
    }
    nextChar(); // read first character
    nextToken(); // read first symbol
    return readFile();
  }
  catch(...)
  {
    return 0;
  }
}

void Parser::nextChar()
{
  if(readBufferPos == readBufferEnd)
  {
    if(currentChar == -1)
      return;
    int i = file.read(readBuffer, sizeof(readBuffer));
    if(i <= 0)
    {
      currentChar = -1;
      return;
    }
    else
    {
      readBufferPos = readBuffer;
      readBufferEnd = readBufferPos + i;
    }
  }
  currentChar = *(readBufferPos++);
}

void Parser::nextToken()
{
  for(;;)
  {
    char c = currentChar;
    nextChar();
    switch(c)
    {
    case -1: currentToken.id = Token::eof; return;
    case '{': currentToken.id = Token::leftBrace; return;
    case '}': currentToken.id = Token::rightBrace; return;
    case ',': currentToken.id = Token::comma; return;
    case ';': currentToken.id = Token::comma; return;
    case '=': currentToken.id = Token::assignment; return;
    case '+': currentToken.id = Token::plus; return;

    case '/': // comment ?
      switch(currentChar)
      {
      case '*': // "/*"
        for(;;)
        {
          nextChar();
          if(currentChar == -1)
            goto handleNextChar;
          if(currentChar == '*')
          {
            nextChar();
            if(currentChar == '/')
            {
              nextChar();
              goto handleNextChar;
            }
          }
        }

      case '/': // "//"
        for(;;)
        {
          nextChar();
          if(currentChar == -1 || currentChar == '\r' || currentChar == '\n')
            goto handleNextChar;
        }

      default:
        unexpectedChar(c);
        goto handleNextChar;
      }

    case '\r':
      ++currentLine;
      if(currentChar == '\n')
        nextChar();
      goto handleNextChar;
    case '\n':
      ++currentLine;
      goto handleNextChar;

    case '"': // string
      {
        currentToken.id = Token::quotedString;
        String& value = currentToken.value;
        value.clear();
        while(currentChar != '"')
        {
          if(currentChar == '\\')
          {
            nextChar();
            if(currentChar == -1)
              goto handleNextChar;
            static const char backslashChars[] = "rnt\"\\";
            static const char backslashTranslation[] = "\r\n\t\"\\";
            const char* str = strchr(backslashChars, currentChar);
            if(str)
              value.append(backslashTranslation[str - backslashChars]);
            else
            {
              value.append('\\');
              value.append(currentChar);
            }
          }
          else
            value.append(currentChar);
          nextChar();
          if(currentChar == -1)
            goto handleNextChar;
        }
        nextChar(); // skip closing "
        return;
      }

    default: // space, keyword or identifier

      if(isspace(c))
        goto handleNextChar;

      if(isalpha(c) || c == '_')
      {
        currentToken.id = Token::string;
        String& value = currentToken.value;
        value.clear();
        value.append(c);
        while(isalnum(currentChar))
        {
          value.append(currentChar);
          nextChar();
        }
        return;
      }

      unexpectedChar(c);
      goto handleNextChar;
    }
handleNextChar: ;
  }
}

void Parser::unexpectedChar(char c)
{
  String message;
  message.format(256, "unexpected character \"%c\"", c);
  errorHandler(errorHandlerUserData, currentLine, message);
  throw false;
}

void Parser::expectToken(Token::Id id)
{
  if(currentToken.id != id)
  {
    String message;
    message.format(256, "expected \"%s\" instead of \"%s\"", currentToken.getName(id), currentToken.getName());
    errorHandler(errorHandlerUserData, currentLine, message);
    throw false;
  }
  nextToken();
}

void Parser::readString(String& string)
{
  switch(currentToken.id)
  {
  case Token::string:
  case Token::quotedString:
    string = currentToken.value;
    nextToken();
    return;
  default:
    {
      String message;
      message.format(256, "expected string instead of \"%s\"", currentToken.getName());
      errorHandler(errorHandlerUserData, currentLine, message);
      throw false;
    }
  }
}

Script* Parser::readFile()
{
  BlockStatement* statement = new BlockStatement(engine);
  while(currentToken.id != Token::eof)
    statement->statements.append(readStatement());
  return new StatementScript(engine, *statement);
}

Statement* Parser::readStatement()
{
  Statement* statement = readAssignment();
  while(currentToken.id == Token::comma)
    nextToken();
  return statement;
}

Statement* Parser::readAssignment()
{
  AssignStatement* statement = new AssignStatement(engine);
  readString(statement->variable);
  if(currentToken.id == Token::assignment)
  {
    nextToken();
    statement->value = readConcatination();
  }
  return statement;
}

Script* Parser::readConcatination()
{
  Statement* statement = readValue();
  while(currentToken.id == Token::plus)
  {
    nextToken();
    BinaryStatement* binaryStatement = new BinaryStatement(engine);
    binaryStatement->leftOperand = statement;
    binaryStatement->rightOperand = readValue();
    statement = binaryStatement;
  }
  return new StatementScript(engine, *statement);
}

Statement* Parser::readValue()
{
  switch(currentToken.id)
  {
  case Token::leftBrace:
    {
      nextToken();
      BlockStatement* statements = new BlockStatement(engine);
      while(currentToken.id != Token::rightBrace)
      {
        if(currentToken.id == Token::eof)
          expectToken(Token::rightBrace);
        statements->statements.append(readStatement());
      }
      nextToken();
      return statements;
    }
  case Token::string:
    {
      ReferenceStatement* statement = new ReferenceStatement(engine);
      readString(statement->variable);
      return statement;
    }
  default:
    {
      StringStatement* statement = new StringStatement(engine);
      readString(statement->value);
      return statement;
    }
  }
}

