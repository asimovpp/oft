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

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

namespace oft {

struct ManualAnnotationSelectionInfo {
  const llvm::Instruction *theInstruction;
};

class ManualAnnotationSelection {
  llvm::SmallVector<llvm::Instruction *, 8> CurInstructions;

public:
  using Result = ManualAnnotationSelectionInfo;

  explicit ManualAnnotationSelection() = default;

  Result perform(const llvm::Module &CurModule);
};

} // namespace oft

