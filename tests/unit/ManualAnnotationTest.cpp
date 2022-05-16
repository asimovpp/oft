#include "OverflowTool/Analysis/ManualAnnotationSelection.hpp"
#include "UnitTestUtils.hpp"
#include "catch2/catch.hpp"

TEST_CASE("No manual annotation") {
    const std::string moduleStr = R"(
define void @f() {
  entry:
    %v0 = alloca i32
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
declare dso_local void @oft_mark_(i8*)

define void @f() {
  entry:
    %v0 = alloca i32
    %castv0 = bitcast i32* %v0 to i8*
    call void @oft_mark_(i8* %castv0)
    ret void
})";

    llvm::LLVMContext ctx;
    auto curMod = parseModule(moduleStr, ctx);

    auto *func = curMod->getFunction("f");
    REQUIRE(func != nullptr);

    oft::ManualAnnotationSelection mas;
    mas.visit(*curMod);
    const auto &res = mas.getAnnotated();
    const auto *marked =
        llvm::dyn_cast<llvm::Instruction>(*(res.values.begin()));

    const auto *expected = &*(func->front().begin());

    REQUIRE(res.values.size() == 1);
    REQUIRE(marked == expected);
}

TEST_CASE("Manual annotation of pointer variable") {
    const std::string moduleStr = R"(
declare dso_local void @oft_mark_(i8*)

define void @f() {
  entry:
    %v0 = alloca double*
    %castv0 = bitcast double** %v0 to i8*
    call void @oft_mark_(i8* %castv0)
    ret void
})";

    llvm::LLVMContext ctx;
    auto curMod = parseModule(moduleStr, ctx);

    auto *func = curMod->getFunction("f");
    REQUIRE(func != nullptr);

    oft::ManualAnnotationSelection mas;
    mas.visit(*curMod);
    const auto &res = mas.getAnnotated();
    const auto *marked =
        llvm::dyn_cast<llvm::Instruction>(*(res.values.begin()));

    const auto *expected = &*(func->front().begin());

    REQUIRE(res.values.size() == 1);
    REQUIRE(marked == expected);
}

TEST_CASE("Manual annotation of function argument") {
    const std::string moduleStr = R"(
declare dso_local void @oft_mark_(i8*)

define void @f(i32 %x) {
  entry:
    %argx.addr = alloca i32
    store i32 %x, i32* %argx.addr
    %castv0 = bitcast i32* %argx.addr to i8*
    call void @oft_mark_(i8* %castv0)
    ret void
})";

    llvm::LLVMContext ctx;
    auto curMod = parseModule(moduleStr, ctx);

    auto *func = curMod->getFunction("f");
    REQUIRE(func != nullptr);
    REQUIRE(func->arg_begin() != func->arg_end());

    llvm::Instruction *expected = &*(func->front().begin());
    llvm::dbgs() << *expected;

    oft::ManualAnnotationSelection mas;
    mas.visit(*curMod);
    const auto &res = mas.getAnnotated();
    const auto *marked =
        llvm::dyn_cast<llvm::Instruction>(*(res.values.begin()));

    REQUIRE(res.values.size() == 1);
    REQUIRE(marked == expected);
}

TEST_CASE("Manual annotation of global variable") {
    const std::string moduleStr = R"(
declare dso_local void @oft_mark_(i8*)
@g = common dso_local global i32 0

define void @f() {
  entry:
    call void @oft_mark_(i8* bitcast (i32* @g to i8*))
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
