// llvm_ast.cpp
#include "llvm_ast.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h" // For dyn_cast, AllocaInst, etc.
#include <vector>

// Define the global symbol table.
std::unordered_map<std::string, llvm::Value *> NamedValues;

// VarDeclNode codegen.
llvm::Value *VarDeclNode::codegen() {
  llvm::Type *llvmType = nullptr;
  if (type == "int")
    llvmType = llvm::Type::getInt32Ty(TheContext);
  else if (type == "float")
    llvmType = llvm::Type::getFloatTy(TheContext);
  else {
    std::cerr << "Unknown type: " << type << std::endl;
    exit(1);
  }

  // Create an alloca in the entry block of the current function.
  llvm::Function *func = Builder.GetInsertBlock()->getParent();
  llvm::IRBuilder<> tmpB(&func->getEntryBlock(), func->getEntryBlock().begin());
  llvm::AllocaInst *alloca = tmpB.CreateAlloca(llvmType, nullptr, name.c_str());

  if (value) {
    llvm::Value *initVal = value->codegen();
    if (!initVal) {
      std::cerr << "Error in codegen for initializer of variable " << name
                << std::endl;
      exit(1);
    }
    Builder.CreateStore(initVal, alloca);
  }

  // Save the alloca for later lookup.
  NamedValues[name] = alloca;
  return alloca;
}

// NumberNode codegen.
llvm::Value *NumberNode::codegen() {
  if (inferredType == "int") {
    return llvm::ConstantInt::get(TheContext,
                                  llvm::APInt(32, std::stoi(value)));
  } else if (inferredType == "float") {
    return llvm::ConstantFP::get(TheContext, llvm::APFloat(std::stof(value)));
  } else {
    std::cerr << "Unsupported number type: " << inferredType << std::endl;
    exit(1);
  }
}

// AssignmentNode codegen.
llvm::Value *AssignmentNode::codegen() {
  llvm::Value *rhs = value->codegen();
  if (!rhs) {
    std::cerr << "Error in codegen for assignment to " << name << std::endl;
    exit(1);
  }
  llvm::Value *var = NamedValues[name];
  if (!var) {
    std::cerr << "Error: Variable '" << name << "' not declared." << std::endl;
    exit(1);
  }
  Builder.CreateStore(rhs, var);
  return rhs;
}

// VariableNode codegen.
// Instead of using the deprecated or unavailable overload of CreateLoad,
// we explicitly get the allocated type from the AllocaInst.
llvm::Value *VariableNode::codegen() {
  llvm::Value *var = NamedValues[name];
  if (!var) {
    std::cerr << "Error: Unknown variable name " << name << std::endl;
    exit(1);
  }
  llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(var);
  if (!allocaInst) {
    std::cerr << "Error: Variable " << name << " is not an alloca."
              << std::endl;
    exit(1);
  }
  llvm::Type *allocatedType = allocaInst->getAllocatedType();
  return Builder.CreateLoad(allocatedType, var, name);
}

// PrintNode codegen.
// Generates a call to printf with a proper format string based on the
// expression type.
llvm::Value *PrintNode::codegen() {
  llvm::Value *exprVal = expr->codegen();
  if (!exprVal) {
    std::cerr << "Error in print: expression codegen returned nullptr."
              << std::endl;
    exit(1);
  }
  llvm::Type *exprType = exprVal->getType();

  // Declare (or get) printf: int printf(i8*, ...);
  llvm::Function *printf_func = TheModule.getFunction("printf");
  if (!printf_func) {
    std::vector<llvm::Type *> printf_arg_types;
    printf_arg_types.push_back(
        llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(TheContext)));
    llvm::FunctionType *printf_type = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(TheContext), printf_arg_types, true);
    printf_func = llvm::Function::Create(
        printf_type, llvm::Function::ExternalLinkage, "printf", TheModule);
  }

  llvm::Value *formatStrPtr = nullptr;
  llvm::Value *printVal = exprVal;
  llvm::Constant *zero =
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(TheContext), 0);
  llvm::Constant *indices[] = {zero, zero};

  if (exprType->isIntegerTy()) {
    // Format string for integer: "%d\n"
    llvm::Constant *formatStr =
        llvm::ConstantDataArray::getString(TheContext, "%d\n", true);
    llvm::GlobalVariable *formatVar = new llvm::GlobalVariable(
        TheModule, formatStr->getType(), true,
        llvm::GlobalValue::PrivateLinkage, formatStr, ".str_int");
    formatStrPtr = llvm::ConstantExpr::getGetElementPtr(
        formatStr->getType(), formatVar,
        llvm::ArrayRef<llvm::Constant *>(indices, 2));
  } else if (exprType->isFloatTy()) {
    // Promote float to double.
    printVal = Builder.CreateFPExt(exprVal, llvm::Type::getDoubleTy(TheContext),
                                   "fpext");
    // Format string for float: "%f\n"
    llvm::Constant *formatStr =
        llvm::ConstantDataArray::getString(TheContext, "%f\n", true);
    llvm::GlobalVariable *formatVar = new llvm::GlobalVariable(
        TheModule, formatStr->getType(), true,
        llvm::GlobalValue::PrivateLinkage, formatStr, ".str_float");
    formatStrPtr = llvm::ConstantExpr::getGetElementPtr(
        formatStr->getType(), formatVar,
        llvm::ArrayRef<llvm::Constant *>(indices, 2));
  } else if (exprType->isDoubleTy()) {
    // Format string for double: "%f\n"
    llvm::Constant *formatStr =
        llvm::ConstantDataArray::getString(TheContext, "%f\n", true);
    llvm::GlobalVariable *formatVar = new llvm::GlobalVariable(
        TheModule, formatStr->getType(), true,
        llvm::GlobalValue::PrivateLinkage, formatStr, ".str_double");
    formatStrPtr = llvm::ConstantExpr::getGetElementPtr(
        formatStr->getType(), formatVar,
        llvm::ArrayRef<llvm::Constant *>(indices, 2));
  } else {
    std::cerr << "Unsupported type in print." << std::endl;
    exit(1);
  }

  return Builder.CreateCall(printf_func, {formatStrPtr, printVal});
}