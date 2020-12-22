//
//
//

#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/InstVisitor.h"
// using llvm::InstVisitor

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/ADT/SmallPtrSet.h"
// using llvm::SmallPtrSet

namespace oft {

constexpr auto ManualAnnotationFnName = "oft_mark";
constexpr auto ManualAnnotationFnArgsNum = 1u;

struct ManualAnnotationSelectionInfo {
  llvm::SmallPtrSet<const llvm::Instruction *, 8> instructions;
};

class ManualAnnotationSelection
    : public llvm::InstVisitor<ManualAnnotationSelection> {
  friend class llvm::InstVisitor<ManualAnnotationSelection>;

  llvm::SmallVector<llvm::Instruction *, 16> CurInstructions;

public:
  using Result = ManualAnnotationSelectionInfo;

  explicit ManualAnnotationSelection() = default;
  Result getAnnotated();
  void visitCallInst(llvm::CallInst &CInst);
  void reset() { CurInstructions.clear(); }
};

} // namespace oft

