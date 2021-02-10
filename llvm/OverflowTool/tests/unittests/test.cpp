#include "catch2/catch.hpp"

#include "OverflowTool/Analysis/ManualAnnotationSelection.hpp"

#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include <iterator>
#include <memory>
#include <string>

unsigned int Factorial(unsigned int number) {
  return number <= 1 ? number : Factorial(number - 1) * number;
}

TEST_CASE("Factorials are computed", "[factorial],[example]") {
  REQUIRE(Factorial(1) == 1);
  REQUIRE(Factorial(2) == 2);
  REQUIRE(Factorial(3) == 6);
}

auto parseModule(const std::string ModuleStr, llvm::LLVMContext &Ctx) {
  llvm::SMDiagnostic err;
  return llvm::parseAssemblyString(ModuleStr, err, Ctx);
}

TEST_CASE("No manual annotation", "[llvmir],[manual],[annotation]") {
  const std::string moduleStr = R"(
define void @f() {
  entry:
    %v0 = alloca i32, align 4
    ret void
})";

  llvm::LLVMContext ctx;
  unsigned expectedAnnotatedNum = 0;
  auto curMod = parseModule(moduleStr, ctx);

  llvm::Function *func = curMod->getFunction("f");
  REQUIRE(func != nullptr);

  oft::ManualAnnotationSelection mas;
  mas.visit(*curMod);
  const auto &res = mas.getAnnotated();

  REQUIRE(res.values.size() == 0);
}

