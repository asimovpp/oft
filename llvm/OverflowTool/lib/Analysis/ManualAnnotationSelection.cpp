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

void ManualAnnotationSelection::visitCallInst(llvm::CallInst &CInst) {
  auto *func = CInst.getFunction();

  if (!func) {
    return;
  }

  if (func->getName() != ManualAnnotationFnName) {
    return;
  }

  assert(func->arg_size() == ManualAnnotationFnArgsNum &&
         "mismatched number of arguments in manual annotation function");

  // TODO see how to handle llvm::Values in general, e.g., globals
  auto *op0 = llvm::dyn_cast<llvm::Instruction>(func->getOperand(0));
  assert(!op0 && "expecting instruction as function argument");

  CurInstructions.push_back(op0);
}

ManualAnnotationSelection::Result ManualAnnotationSelection::getAnnotated() {
  ManualAnnotationSelection::Result res;
  res.instructions.insert(CurInstructions.begin(), CurInstructions.end());
  return res;
}

} // namespace oft
