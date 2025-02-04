// llvm_parser.h
#ifndef LLVM_PARSER_H
#define LLVM_PARSER_H

#include "llvm_ast.h"
#include "llvm_lexer.cpp" // For simplicity; ideally, you’d separate declarations.
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class Parser {
private:
  Lexer &lexer;
  Token currentToken;
  std::unordered_map<std::string, std::string> symbolTable;

  void nextToken() { currentToken = lexer.nextToken(); }

public:
  Parser(Lexer &lexer) : lexer(lexer) { nextToken(); }

  // Parse a number.
  ASTNode *parseNumber() {
    std::string val = currentToken.value;
    std::string type = (val.find('.') != std::string::npos) ? "float" : "int";
    ASTNode *node = new NumberNode(val, type);
    nextToken();
    return node;
  }

  // Parse a variable declaration: var int x = 5;
  ASTNode *parseVarDecl() {
    nextToken();                           // Consume 'var'
    std::string type = currentToken.value; // Expect type (int or float)
    nextToken();                           // Consume type
    std::string name = currentToken.value; // Expect identifier
    nextToken();                           // Consume identifier

    if (symbolTable.find(name) != symbolTable.end()) {
      std::cerr << "Error: Variable '" << name << "' already declared."
                << std::endl;
      exit(1);
    }
    symbolTable[name] = type;

    ASTNode *init = nullptr;
    if (currentToken.type == TokenType::ASSIGN) {
      nextToken(); // Consume '='
      init = parseNumber();
    }

    if (currentToken.type != TokenType::SEMICOLON) {
      std::cerr << "Expected ';' after variable declaration." << std::endl;
      exit(1);
    }
    nextToken(); // Consume ';'
    return new VarDeclNode(type, name, init);
  }

  // Parse an assignment: x = 3;
  ASTNode *parseAssignment() {
    std::string name = currentToken.value;
    nextToken(); // Consume identifier

    if (currentToken.type != TokenType::ASSIGN) {
      std::cerr << "Error: Expected '=' in assignment." << std::endl;
      exit(1);
    }
    nextToken(); // Consume '='

    ASTNode *expr = parseNumber(); // For now, only numbers are supported.
    if (currentToken.type != TokenType::SEMICOLON) {
      std::cerr << "Expected ';' after assignment." << std::endl;
      exit(1);
    }
    nextToken(); // Consume ';'
    return new AssignmentNode(name, expr);
  }

  // Parse a print statement: print(expr);
  ASTNode *parsePrint() {
    nextToken(); // Consume 'print'
    if (currentToken.type != TokenType::LPAREN) {
      std::cerr << "Expected '(' after 'print'" << std::endl;
      exit(1);
    }
    nextToken(); // Consume '('
    ASTNode *expr = nullptr;
    if (currentToken.type == TokenType::NUMBER) {
      expr = parseNumber();
    } else if (currentToken.type == TokenType::IDENTIFIER) {
      std::string name = currentToken.value;
      nextToken();
      expr = new VariableNode(name);
    } else {
      std::cerr << "Unexpected token in print expression: "
                << currentToken.value << std::endl;
      exit(1);
    }
    if (currentToken.type != TokenType::RPAREN) {
      std::cerr << "Expected ')' after expression in print statement"
                << std::endl;
      exit(1);
    }
    nextToken(); // Consume ')'
    if (currentToken.type != TokenType::SEMICOLON) {
      std::cerr << "Expected ';' after print statement" << std::endl;
      exit(1);
    }
    nextToken(); // Consume ';'
    return new PrintNode(expr);
  }

  // Parse the entire input.
  std::vector<ASTNode *> parse() {
    std::vector<ASTNode *> nodes;
    while (currentToken.type != TokenType::END) {
      if (currentToken.type == TokenType::VAR)
        nodes.push_back(parseVarDecl());
      else if (currentToken.type == TokenType::PRINT)
        nodes.push_back(parsePrint());
      else if (currentToken.type == TokenType::IDENTIFIER)
        nodes.push_back(parseAssignment());
      else {
        std::cerr << "Unexpected token: " << currentToken.value << std::endl;
        exit(1);
      }
    }
    return nodes;
  }
};

#endif // LLVM_PARSER_H