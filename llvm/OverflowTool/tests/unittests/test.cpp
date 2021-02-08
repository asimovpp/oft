// This tells Catch to provide a main() - do this once in a single source file
#define CATCH_CONFIG_MAIN

#include "catch2/catch.hpp"

#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

unsigned int Factorial(unsigned int number) {
  return number <= 1 ? number : Factorial(number - 1) * number;
}

TEST_CASE("Factorials are computed", "[factorial]") {
  REQUIRE(Factorial(1) == 1);
  REQUIRE(Factorial(2) == 2);
  REQUIRE(Factorial(3) == 6);
  REQUIRE(Factorial(10) == 3628800);
}

TEST_CASE("Manual annotation", "[llvmir],[manual],[annotation]") {
  llvm::LLVMContext C;

  const char *ModuleString = R"(
@g = common dso_local global i32 0

define void @f(i32 %x) {
  entry:
    %v0 = alloca i32, align 4
    %v1 = add i32 1, 1
    %v2 = add i32 %x, 2
    ret void
})";

  llvm::SMDiagnostic Err;
  std::unique_ptr<llvm::Module> M =
      llvm::parseAssemblyString(ModuleString, Err, C);
  llvm::Function *F = M->getFunction("f");

  REQUIRE(F != nullptr);
  REQUIRE(F->arg_begin() != F->arg_end());

  llvm::BasicBlock &entryBB = F->front();
  auto it = entryBB.begin();
  llvm::Instruction &I0 = *(it);
  llvm::Instruction &I1 = *(++it);

  llvm::Argument &X = *F->arg_begin();
}
