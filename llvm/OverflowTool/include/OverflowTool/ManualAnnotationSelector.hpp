//
//
//

#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

namespace llvm {
class Function;
} // namespace llvm

namespace ovt {

class ManualAnnotationSelector {
  llvm::SmallVector<llvm::Instruction *, 8> CurInstructions;

public:
  explicit ManualAnnotationSelector(const llvm::Function &CurFunc);
};

} // namespace ovt

