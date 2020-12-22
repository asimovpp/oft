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

struct ManualAnnotationSelectionInfo {
  llvm::SmallPtrSet<const llvm::Instruction *, 8> instructions;
};

class ManualAnnotationSelection
    : public llvm::InstVisitor<ManualAnnotationSelection> {
  friend class llvm::InstVisitor<ManualAnnotationSelection>;

  llvm::SmallVector<llvm::Instruction *, 16> curInstructions;

public:
  using Result = ManualAnnotationSelectionInfo;

  explicit ManualAnnotationSelection() = default;
  Result getAnnotated();
  void visitInstruction(llvm::Instruction &Inst);
  void reset() { curInstructions.clear(); }
};

} // namespace oft

