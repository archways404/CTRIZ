// llvm_lexer.cpp
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

// Token types
enum class TokenType {
  VAR,
  INT,
  FLOAT,
  PRINT, // New token for print keyword
  IDENTIFIER,
  NUMBER,
  ASSIGN,
  SEMICOLON,
  PLUS,
  MINUS,
  MULT,
  DIV,
  LPAREN,
  RPAREN,
  END
};

// Token structure
struct Token {
  TokenType type;
  std::string value;
};

class Lexer {
private:
  std::string source;
  size_t index = 0;

public:
  Lexer(const std::string &src) : source(src) {}

  char peek() { return (index < source.length()) ? source[index] : '\0'; }
  char advance() { return (index < source.length()) ? source[index++] : '\0'; }

  void skipWhitespace() {
    while (isspace(peek()))
      advance();
  }

  Token nextToken() {
    skipWhitespace();
    char current = peek();

    if (isdigit(current)) {
      std::string num;
      while (isdigit(peek()))
        num += advance();

      if (peek() == '.') {
        num += advance(); // Consume '.'
        if (!isdigit(peek())) {
          std::cerr << "Error: Invalid float syntax" << std::endl;
          exit(1);
        }
        while (isdigit(peek()))
          num += advance();
      }
      return {TokenType::NUMBER, num};
    }

    if (isalpha(current)) {
      std::string identifier;
      while (isalnum(peek()))
        identifier += advance();

      if (identifier == "var")
        return {TokenType::VAR, identifier};
      if (identifier == "int")
        return {TokenType::INT, identifier};
      if (identifier == "float")
        return {TokenType::FLOAT, identifier};
      if (identifier == "print")
        return {TokenType::PRINT, identifier};

      return {TokenType::IDENTIFIER, identifier};
    }

    switch (current) {
    case '=':
      advance();
      return {TokenType::ASSIGN, "="};
    case ';':
      advance();
      return {TokenType::SEMICOLON, ";"};
    case '+':
      advance();
      return {TokenType::PLUS, "+"};
    case '-':
      advance();
      return {TokenType::MINUS, "-"};
    case '*':
      advance();
      return {TokenType::MULT, "*"};
    case '/':
      advance();
      return {TokenType::DIV, "/"};
    case '(':
      advance();
      return {TokenType::LPAREN, "("};
    case ')':
      advance();
      return {TokenType::RPAREN, ")"};
    case '\0':
      return {TokenType::END, ""};
    default:
      std::cerr << "Unexpected character: " << current << std::endl;
      exit(1);
    }
  }
};