//
//
//

#pragma once

#include "SDCManualAnnotation/Config.hpp"

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

namespace llvm {
class Function;
} // namespace llvm

namespace sdc {

class ManualAnnotationSelector {
  llvm::SmallVector<llvm::Instruction *, 8> CurInstructions;

public:
  explicit ManualAnnotationSelector(const llvm::Function &CurFunc);
};

} // namespace sdc

