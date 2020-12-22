//
//
//

#include "OverflowTool/Analysis/ManualAnnotationSelection.hpp"

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/Debug.h"
// using LLVM_DEBUG macro
// using llvm::dbgs

#define DEBUG_TYPE "oft-manual-annotation-selection-analysis"

namespace oft {

void ManualAnnotationSelection::visitInstruction(llvm::Instruction &Inst) {}

ManualAnnotationSelection::Result ManualAnnotationSelection::getAnnotated() {
  ManualAnnotationSelection::Result res;
  res.instructions.insert(curInstructions.begin(), curInstructions.end());
  return res;
}

} // namespace oft
