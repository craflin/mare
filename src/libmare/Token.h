
#pragma once

#include "Tools/String.h"

class Token
{
public:
  enum Id
  {
    leftParenthesis, rightParenthesis, leftBrace, rightBrace, comma,
    assignment, plusAssignment, minusAssignment,
    plus, minus, not_,
    interrogation, colon,
    and_, or_,
    equal, notEqual, greaterThan, lowerThan, greaterEqualThan, lowerEqualThan,
    string, quotedString,
    eof,
    numOfTokens,
  };
  Id id;
  String value;

  const char* getName(Id id) const
  {
    static const char* names[] = {
      "(", ")", "{", "}", ",",
      "=", "+=", "-=",
      "+", "-", "!", 
      "?", ":",
      "&&", "||",
      "==", "!=", ">", "<", ">=", "<=",
      "string", "string",
      "end of file"
    };
    return names[id];
  }
  const char* getName() const {return getName(id);}
};
