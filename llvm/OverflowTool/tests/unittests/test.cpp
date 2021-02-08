// This tells Catch to provide a main() - do this once in a single source file
#define CATCH_CONFIG_MAIN

#include "catch2/catch.hpp"

#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include <string>

unsigned int Factorial(unsigned int number) {
  return number <= 1 ? number : Factorial(number - 1) * number;
}

TEST_CASE("Factorials are computed", "[factorial],[example]") {
  REQUIRE(Factorial(1) == 1);
  REQUIRE(Factorial(2) == 2);
  REQUIRE(Factorial(3) == 6);
}

TEST_CASE("Manual annotation", "[llvmir],[manual],[annotation]") {
  llvm::LLVMContext ctx;

  const std::string moduleStringStart = R"(
@g = common dso_local global i32 0

define void @f(i32 %x) {
  entry:
    %v0 = alloca i32, align 4
    %v1 = add i32 1, 1
    %v2 = add i32 %x, 2)";

  const std::string moduleStringEnd = R"(
    ret void
})";

  llvm::SMDiagnostic err;

  SECTION("annotating stack variable") {
    const std::string moduleStringAnnotations = R"(
    %castv0 = bitcast i32* %v0 to i8*
)";

    auto moduleString =
        moduleStringStart + moduleStringAnnotations + moduleStringEnd;

    std::unique_ptr<llvm::Module> curMod =
        llvm::parseAssemblyString(moduleString, err, ctx);
    llvm::Function *func = curMod->getFunction("f");

    REQUIRE(func != nullptr);
    REQUIRE(func->arg_begin() != func->arg_end());

    llvm::BasicBlock &entryBB = func->front();
    auto it = entryBB.begin();
    llvm::Instruction &i0 = *(it);
    llvm::Instruction &i1 = *(++it);

    llvm::Argument &x = *func->arg_begin();
  }
}
