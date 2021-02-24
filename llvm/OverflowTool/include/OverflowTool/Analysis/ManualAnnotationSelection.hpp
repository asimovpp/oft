//
//
//

#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/Value.h"
// using llvm::Value

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/InstVisitor.h"
// using llvm::InstVisitor

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/ADT/SmallPtrSet.h"
// using llvm::SmallPtrSet

namespace oft {

constexpr auto ManualAnnotationFnName = "oft_mark_";
constexpr auto ManualAnnotationFnArgsNum = 1u;

struct mark_entry_t {
    std::string fnName;
    bool retVal;
    std::vector<unsigned> fnArgs;

    mark_entry_t(std::string fnName, bool retVal,
                 const std::vector<unsigned> &fnArgs)
        : fnName(std::move(fnName)), retVal(retVal), fnArgs(fnArgs) {}
};

using mark_db_t = std::vector<mark_entry_t>;

struct ManualAnnotationSelectionInfo {
    llvm::SmallPtrSet<const llvm::Value *, 8> values;
};

class ManualAnnotationSelection
    : public llvm::InstVisitor<ManualAnnotationSelection> {
    friend class llvm::InstVisitor<ManualAnnotationSelection>;

    llvm::SmallVector<llvm::Value *, 16> Annotated;

    mark_db_t MarkedDB;

  public:
    using Result = ManualAnnotationSelectionInfo;

    explicit ManualAnnotationSelection();
    Result getAnnotated();
    void visitCallInst(llvm::CallInst &CInst);

    void visitDefaultAnnotationFunc(llvm::CallInst &CInst);
    void visitCustomFunc(llvm::CallInst &CInst, llvm::StringRef FuncName);

    void reset() { Annotated.clear(); }
};

} // namespace oft
