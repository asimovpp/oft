//

#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

namespace oft {

constexpr auto ManualAnnotationFnName = "oft_mark_";
constexpr auto ManualAnnotationFnArgsNum = 1u;

struct annotation_entry_t {
    std::string fnName;
    bool retVal;
    std::vector<unsigned> fnArgs;

    annotation_entry_t(std::string fnName, bool retVal,
                       const std::vector<unsigned> &fnArgs)
        : fnName(std::move(fnName)), retVal(retVal), fnArgs(fnArgs) {}
};

using annotation_db_t = std::vector<annotation_entry_t>;

struct ManualAnnotationSelectionInfo {
    llvm::SmallPtrSet<const llvm::Value *, 8> values;
};

class ManualAnnotationSelection
    : public llvm::InstVisitor<ManualAnnotationSelection> {
    friend class llvm::InstVisitor<ManualAnnotationSelection>;

    llvm::SmallVector<llvm::Value *, 16> Annotated;

    annotation_db_t AnnotationDB;

  public:
    using Result = ManualAnnotationSelectionInfo;

    explicit ManualAnnotationSelection() = default;

    template <typename IteratorT>
    explicit ManualAnnotationSelection(IteratorT begin, IteratorT end) {
        AnnotationDB.insert(AnnotationDB.end(), begin, end);
    }

    Result getAnnotated();
    void visitCallInst(llvm::CallInst &CInst);

    void visitDefaultAnnotationFunc(llvm::CallInst &CInst);
    void visitCustomFunc(llvm::CallInst &CInst, llvm::StringRef FuncName);

    void reset() { Annotated.clear(); }
};

} // namespace oft
