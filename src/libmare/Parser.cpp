
#include <cctype>
#include <cstring>

#include "Parser.h"
#include "Tools/String.h"
#include "Tools/Error.h"
#include "Statement.h"

Parser::Parser(Engine& engine) : engine(engine), includeFile(0), readBufferPos(0), readBufferEnd(0), currentLine(1), currentChar(' ') {}

Statement* Parser::parse(const String& file, Engine::ErrorHandler errorHandler, void* userData)
{
  this->errorHandler = errorHandler;
  this->errorHandlerUserData = userData;
  this->filePath = file;

  try
  {
    if(!this->file.open(file))
    {
      errorHandler(errorHandlerUserData, file, 0, Error::getString());
      throw false;
    }
    includeFile = new IncludeFile(engine);
    includeFile->fileDir = File::getDirname(file);
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
    if(currentChar == '\0')
      return;
    size_t i = file.read(readBuffer, sizeof(readBuffer));
    if(i == 0)
    {
      currentChar = '\0';
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
    case '\0': currentToken.id = Token::eof; return;
    case '(': currentToken.id = Token::leftParenthesis; return;
    case ')': currentToken.id = Token::rightParenthesis; return;
    case '{': currentToken.id = Token::leftBrace; return;
    case '}': currentToken.id = Token::rightBrace; return;
    case ',': currentToken.id = Token::comma; return;
    case ';': currentToken.id = Token::comma; return;
    case '?': currentToken.id = Token::interrogation; return;
    case ':': currentToken.id = Token::colon; return;

    case '&':
      if(currentChar == '&')
      {
        nextChar();
        currentToken.id = Token::and_;
      }
      else
        unexpectedChar(c);
      return;

    case '|':
      if(currentChar == '|')
      {
        nextChar();
        currentToken.id = Token::or_;
      }
      else
        unexpectedChar(c);
      return;

    case '=':
      if(currentChar == '=')
      {
        nextChar();
        currentToken.id = Token::equal;
      }
      else
        currentToken.id = Token::assignment;
      return;

    case '+':
      if(currentChar == '=')
      {
        nextChar();
        currentToken.id = Token::plusAssignment;
      }
      else
        currentToken.id = Token::plus;
      return;

    case '-':
      if(currentChar == '=')
      {
        nextChar();
        currentToken.id = Token::minusAssignment;
      }
      else
        currentToken.id = Token::minus;
      return;

    case '!':
      if(currentChar == '=')
      {
        nextChar();
        currentToken.id = Token::notEqual;
      }
      else
        currentToken.id = Token::not_;
      return;

    case '>':
      if(currentChar == '=')
      {
        nextChar();
        currentToken.id = Token::greaterEqualThan;
      }
      else
        currentToken.id = Token::greaterThan;
      return;

    case '<':
      if(currentChar == '=')
      {
        nextChar();
        currentToken.id = Token::lowerEqualThan;
      }
      else
        currentToken.id = Token::lowerThan;
      return;

    case '/': // comment ?
      switch(currentChar)
      {
      case '*': // "/*"
        for(;;)
        {
          nextChar();
          if(currentChar == '\0')
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
          if(currentChar == '\0' || currentChar == '\r' || currentChar == '\n')
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
            if(currentChar == '\0')
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
          if(currentChar == '\0')
            goto handleNextChar;
        }
        nextChar(); // skip closing "
        return;
      }

    default: // space, keyword or identifier

      if(isspace(*(unsigned char*)&c))
        goto handleNextChar;

      if(isalpha(c) || c == '_')
      {
        currentToken.id = Token::string;
        String& value = currentToken.value;
        value.clear();
        value.append(c);
        while(isalnum(currentChar) || currentChar == '_')
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
  errorHandler(errorHandlerUserData, filePath, currentLine, message);
  throw false;
}

void Parser::expectToken(Token::Id id)
{
  if(currentToken.id != id)
  {
    String message;
    message.format(256, "expected \"%s\" instead of \"%s\"", currentToken.getName(id), currentToken.getName());
    errorHandler(errorHandlerUserData, filePath, currentLine, message);
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
      errorHandler(errorHandlerUserData, filePath, currentLine, message);
      throw false;
    }
  }
}

Statement* Parser::readFile()
{
  BlockStatement* statement = new BlockStatement(*includeFile);
  while(currentToken.id != Token::eof)
    statement->statements.append(readStatement());
  return statement;
}

Statement* Parser::readStatements()
{
  if(currentToken.id == Token::leftBrace)
  {
    nextToken();
    BlockStatement* blockStatement = new BlockStatement(*includeFile);
    while(currentToken.id != Token::rightBrace)
      blockStatement->statements.append(readStatement());
    nextToken();
    return blockStatement;
  }
  else
    return readStatement();
}

Statement* Parser::readStatement()
{
  Statement* statement;
  if(currentToken.id == Token::string && currentToken.value == "if")
  {
    nextToken();
    IfStatement* ifStatement = new IfStatement(*includeFile);
    ifStatement->condition = readExpression();
    ifStatement->thenStatements = readStatements();
    if(currentToken.id == Token::string && currentToken.value == "else")
    {
      nextToken();
      ifStatement->elseStatements = readStatements();
    }
    statement = ifStatement;
  }
  else if(currentToken.id == Token::string && currentToken.value == "include")
  {
    nextToken();

    String fileName;
    readString(fileName);
    if(!File::isPathAbsolute(fileName))
    {
      String parentDir = includeFile->fileDir;
      if(parentDir != ".")
        fileName = parentDir + "/" + fileName;
    }

    Parser parser(engine);
    WrapperStatement* statement = new WrapperStatement(*includeFile);
    statement->statement = parser.parse(fileName, errorHandler, errorHandlerUserData);
    if(!statement->statement)
      throw false;
    return statement;
  }
  else
    statement = readAssignment();
  while(currentToken.id == Token::comma)
    nextToken();
  return statement;
}

Statement* Parser::readAssignment()
{
  if(currentToken.id == Token::minus)
  {
    nextToken();
    RemoveStatement* statement = new RemoveStatement(*includeFile);
    readString(statement->variable);
    return statement;
  }
  else
  {
    AssignStatement* statement = new AssignStatement(*includeFile);
    readString(statement->variable);
    switch(currentToken.id)
    {
    case Token::plusAssignment:
    case Token::minusAssignment:
      statement->operation = currentToken.id;
      // no break
    case Token::assignment:
      nextToken();
      statement->value = readConcatination();
      break;
    default:
      break;
    }
    return statement;
  }
}

Statement* Parser::readExpression()
{
  Statement* statement = readOrForumla();
  if(currentToken.id == Token::interrogation)
  {
    nextToken();
    IfStatement* ifStatement = new IfStatement(*includeFile);
    ifStatement->condition = statement;
    ifStatement->thenStatements = readExpression();
    if(currentToken.id == Token::colon)
    {
      nextToken();
      ifStatement->elseStatements = readExpression();
    }
    return ifStatement;
  }
  return statement;
}

Statement* Parser::readOrForumla()
{
  Statement* statement = readAndFormula();
  while(currentToken.id == Token::or_)
  {
    BinaryStatement* binaryStatement = new BinaryStatement(*includeFile);
    binaryStatement->operation = currentToken.id;
    nextToken();
    binaryStatement->leftOperand = statement;
    binaryStatement->rightOperand = readAndFormula();
    statement = binaryStatement;
  }
  return statement;
}

Statement* Parser::readAndFormula()
{
  Statement* statement = readComparison();
  while(currentToken.id == Token::and_)
  {
    BinaryStatement* binaryStatement = new BinaryStatement(*includeFile);
    binaryStatement->operation = currentToken.id;
    nextToken();
    binaryStatement->leftOperand = statement;
    binaryStatement->rightOperand = readComparison();
    statement = binaryStatement;
  }
  return statement;
}

Statement* Parser::readComparison()
{
  Statement* statement = readRelation();
  while(currentToken.id == Token::equal || currentToken.id == Token::notEqual)
  {
    BinaryStatement* binaryStatement = new BinaryStatement(*includeFile);
    binaryStatement->operation = currentToken.id;
    nextToken();
    binaryStatement->leftOperand = statement;
    binaryStatement->rightOperand = readRelation();
    statement = binaryStatement;
  }
  return statement;
}

Statement* Parser::readRelation()
{
  Statement* statement = readValue();
  while(currentToken.id == Token::greaterThan || currentToken.id == Token::lowerThan || 
    currentToken.id == Token::greaterEqualThan || currentToken.id == Token::lowerEqualThan)
  {
    BinaryStatement* binaryStatement = new BinaryStatement(*includeFile);
    binaryStatement->operation = currentToken.id;
    nextToken();
    binaryStatement->leftOperand = statement;
    binaryStatement->rightOperand = readValue();
    statement = binaryStatement;
  }
  return statement;
}

Statement* Parser::readConcatination()
{
  Statement* statement = readValue();
  while(currentToken.id == Token::plus || currentToken.id == Token::minus)
  {
    BinaryStatement* binaryStatement = new BinaryStatement(*includeFile);
    binaryStatement->operation = currentToken.id;
    nextToken();
    binaryStatement->leftOperand = statement;
    binaryStatement->rightOperand = readValue();
    statement = binaryStatement;
  }
  return statement;
}

Statement* Parser::readValue()
{
  switch(currentToken.id)
  {
  case Token::not_:
    {
      UnaryStatement* statement = new UnaryStatement(*includeFile);
      statement->operation = currentToken.id;
      nextToken();
      statement->operand = readValue();
      return statement;
    }
  case Token::leftParenthesis:
    {
      nextToken();
      Statement* statement = readExpression();
      expectToken(Token::rightParenthesis);
      return statement;
    }
  case Token::leftBrace:
    {
      nextToken();
      BlockStatement* statements = new BlockStatement(*includeFile);
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
    if(currentToken.value == "true")
    {
      nextToken();
      StringStatement* statement = new StringStatement(*includeFile);
      statement->value = "true";
      return statement;
    }
    else if(currentToken.value == "false")
    {
      nextToken();
      return new StringStatement(*includeFile);
    }
    else
    {
      ReferenceStatement* statement = new ReferenceStatement(*includeFile);
      readString(statement->variable);
      return statement;
    }
  default: // quotedString
    {
      StringStatement* statement = new StringStatement(*includeFile);
      readString(statement->value);
      return statement;
    }
  }
}

