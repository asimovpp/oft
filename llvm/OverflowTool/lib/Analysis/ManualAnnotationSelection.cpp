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
  auto *func = CInst.getCalledFunction();

  if (func && func->getName() != ManualAnnotationFnName) {
    return;
  }

  assert(func->arg_size() == ManualAnnotationFnArgsNum &&
         "mismatched number of arguments in manual annotation function");

  // TODO see how to handle llvm::Values in general, e.g., globals
  auto *op0 = llvm::dyn_cast<llvm::Instruction>(*CInst.arg_begin());
  assert(op0 && "expecting instruction as function argument");

  // expect bitcast
  if (auto *bitcastInst = llvm::dyn_cast<llvm::BitCastInst>(op0)) {
    op0 = bitcastInst->getOperand(0);
  }

  Annotated.push_back(op0);
}

ManualAnnotationSelection::Result ManualAnnotationSelection::getAnnotated() {
  ManualAnnotationSelection::Result res;
  res.values.insert(Annotated.begin(), Annotated.end());

  // TODO move this when print<> is implemented
  for (const auto &e : res.values) {
    llvm::dbgs() << *e << '\n';
  }

  return res;
}

} // namespace oft
