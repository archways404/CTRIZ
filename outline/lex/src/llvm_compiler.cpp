// llvm_compiler.cpp
#include "llvm_ast.h"
#include "llvm_parser.h" // This includes the lexer, parser, and AST definitions.
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// Define LLVM globals.
llvm::LLVMContext TheContext;
llvm::IRBuilder<> Builder(TheContext);
llvm::Module TheModule("MyModule", TheContext);

int main(int argc, char **argv) {
  // Check for correct command-line usage.
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <filename.ctz>" << std::endl;
    return 1;
  }

  // Open the file.
  std::ifstream infile(argv[1]);
  if (!infile.is_open()) {
    std::cerr << "Error: could not open file " << argv[1] << std::endl;
    return 1;
  }

  // Read the entire file into a string.
  std::stringstream buffer;
  buffer << infile.rdbuf();
  std::string code = buffer.str();

  // Debug: (optional) Print the source code read from file.
  // std::cout << "Source code:\n" << code << std::endl;

  // Create the lexer and parser.
  Lexer lexer(code);
  Parser parser(lexer);
  std::vector<ASTNode *> astNodes = parser.parse();

  if (astNodes.empty()) {
    std::cerr << "Error: No valid statements found." << std::endl;
    return 1;
  }

  // (Optional) Remove any debug messages from output.
  // std::cout << "Generating LLVM IR..." << std::endl;

  // Create the main function: void main()
  llvm::FunctionType *funcType =
      llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), false);
  llvm::Function *mainFunction = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, "main", TheModule);

  // Create the entry block.
  llvm::BasicBlock *entryBlock =
      llvm::BasicBlock::Create(TheContext, "entry", mainFunction);
  Builder.SetInsertPoint(entryBlock);

  // Generate code for each AST node.
  for (ASTNode *node : astNodes) {
    if (!node->codegen()) {
      std::cerr << "Error: codegen() returned nullptr." << std::endl;
      return 1;
    }
  }

  Builder.CreateRetVoid();

  // Print the generated LLVM IR (to stderr or redirect as needed).
  TheModule.print(llvm::errs(), nullptr);

  return 0;
}