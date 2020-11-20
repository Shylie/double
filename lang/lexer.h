#pragma once

#include <string>

class Token
{
public:
	enum Type : int
	{
		End = -1,

		Def = -2,
		Extern = -3,

		Identifier = -4,
		Number = -5,

		If = -6,
		Then = -7,
		Else = -8,
		ElseIf = -9,

		For = -10,
		Do = -11,

		Binary = -12,
		Unary = -13,

		Var = -14
	};

	Token();
	Token(int type);
	Token(const std::string& identifier);
	Token(double number);

	int type;
	std::string identifier;
	double number;
};

class Lexer
{
public:
	Lexer();
	Lexer(const std::string& code);
	Lexer(const char* code);
	Lexer& operator=(const std::string& code);
	Lexer& operator=(const char* code);

	Token Next();

private:
	std::string code;
	int start, current;

	char Advance();

	char Peek(int offset) const;

	double ParseNumber();
	std::string ParseIdentifier();
};