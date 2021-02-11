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

TEST_CASE("No manual annotation") {
  const std::string moduleStr = R"(
define void @f() {
  entry:
    %v0 = alloca i32, align 4
    ret void
})";

  llvm::LLVMContext ctx;
  auto curMod = parseModule(moduleStr, ctx);

  auto *func = curMod->getFunction("f");
  REQUIRE(func != nullptr);

  oft::ManualAnnotationSelection mas;
  mas.visit(*curMod);
  const auto &res = mas.getAnnotated();

  REQUIRE(res.values.size() == 0);
}

TEST_CASE("Manual annotation of stack variable") {
  const std::string moduleStr = R"(
declare dso_local void @oft_mark(i8*)

define void @f() {
  entry:
    %v0 = alloca i32, align 4
    %castv0 = bitcast i32* %v0 to i8*
    call void @oft_mark(i8* %castv0)
    ret void
})";

  llvm::LLVMContext ctx;
  auto curMod = parseModule(moduleStr, ctx);

  auto *func = curMod->getFunction("f");
  REQUIRE(func != nullptr);

  oft::ManualAnnotationSelection mas;
  mas.visit(*curMod);
  const auto &res = mas.getAnnotated();
  const auto *marked = llvm::dyn_cast<llvm::Instruction>(*(res.values.begin()));

  const auto *expected = &*(func->front().begin());

  REQUIRE(res.values.size() == 1);
  REQUIRE(marked == expected);
}

TEST_CASE("Manual annotation of global variable") {
  const std::string moduleStr = R"(
declare dso_local void @oft_mark(i8*)
@g = common dso_local global i32 0

define void @f() {
  entry:
    call void @oft_mark(i8* bitcast (i32* @g to i8*))
    ret void
})";

  llvm::LLVMContext ctx;
  auto curMod = parseModule(moduleStr, ctx);

  auto *expected = curMod->getGlobalVariable("g");
  REQUIRE(expected != nullptr);

  oft::ManualAnnotationSelection mas;
  mas.visit(*curMod);
  const auto &res = mas.getAnnotated();
  const auto *marked =
      llvm::dyn_cast<llvm::GlobalVariable>(*(res.values.begin()));

  REQUIRE(res.values.size() == 1);
  REQUIRE(marked == expected);
}
