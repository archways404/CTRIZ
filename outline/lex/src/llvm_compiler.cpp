// llvm_compiler.cpp
#include "llvm_ast.h"
#include "llvm_parser.h" // This includes lexer and AST definitions.
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <iostream>
#include <vector>

// Define LLVM globals.
llvm::LLVMContext TheContext;
llvm::IRBuilder<> Builder(TheContext);
llvm::Module TheModule("MyModule", TheContext);

int main() {
  // Source code with print statements.
  std::string code =
      "var int x = 5; var float y = 10.5; x = 3; print(x); print(y);";

  // Create the lexer and parser.
  Lexer lexer(code);
  Parser parser(lexer);
  std::vector<ASTNode *> astNodes = parser.parse();

  if (astNodes.empty()) {
    std::cerr << "Error: No valid statements found." << std::endl;
    return 1;
  }

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

  // Print the generated LLVM IR.
  TheModule.print(llvm::errs(), nullptr);

  return 0;
}