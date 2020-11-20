#include "lexer.h"

Token::Token() : Token('\0') { }
Token::Token(int type) : type(type), number(0.0) { }
Token::Token(const std::string& identifier) : type(Identifier), identifier(identifier), number(0.0) { }
Token::Token(double number) : type(Number), number(number) { }

static bool IsSpace(char c)
{
	return c == ' ' || c == '\r' || c == '\t' || c == '\n';
}

static bool IsDigit(char c)
{
	return c >= '0' && c <= '9';
}

static bool IsAlpha(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

Lexer::Lexer() : Lexer("") { }

Lexer::Lexer(const std::string& code) : code(code), start(0), current(0)
{
}

Lexer::Lexer(const char* code) : code(code), start(0), current(0)
{
}

Lexer& Lexer::operator=(const std::string& code)
{
	this->code = code;
	start = current = 0;

	return *this;
}

Lexer& Lexer::operator=(const char* code)
{
	this->code = code;
	start = current = 0;

	return *this;
}

Token Lexer::Next()
{
	while (IsSpace(Peek(0)))
	{
		Advance();
	}

	if (Peek(0) == '/' && Peek(1) == '/')
	{
		Advance();
		Advance();
		while (Peek(0) != '\n')
		{
			Advance();
		}
		while (IsSpace(Peek(0)))
		{
			Advance();
		}
	}

	start = current;

	if (IsDigit(Peek(0)))
	{
		return ParseNumber();
	}

	if (IsAlpha(Peek(0)))
	{
		std::string str = ParseIdentifier();
		if (str == "def") { return Token::Def; }
		if (str == "extern") { return Token::Extern; }
		if (str == "end") { return Token::End; }
		if (str == "if") { return Token::If; }
		if (str == "then") { return Token::Then; }
		if (str == "else") { return Token::Else; }
		if (str == "elseif") { return Token::ElseIf; }
		if (str == "for") { return Token::For; }
		if (str == "do") { return Token::Do; }
		if (str == "binary") { return Token::Binary; }
		if (str == "unary") { return Token::Unary; }
		if (str == "var") { return Token::Var; }
		return str;
	}

	return Advance();
}

char Lexer::Advance()
{
	return current >= code.length() ? '\0' : code[current++];
}

char Lexer::Peek(int offset) const
{
	return (current + offset < 0 || current + offset >= code.length()) ? '\0' : code[current + offset];
}

double Lexer::ParseNumber()
{
	while (IsDigit(Peek(0))) { Advance(); }

	if (Peek(0) == '.')
	{
		Advance();
		
		while (IsDigit(Peek(0))) { Advance(); }
	}

	return strtod(code.substr(start, current - start).c_str(), nullptr);
}

std::string Lexer::ParseIdentifier()
{
	while (IsAlpha(Peek(0))) { Advance(); }

	return code.substr(start, current - start);
}