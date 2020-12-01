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

ManualAnnotationSelection::Result
ManualAnnotationSelection::perform(const llvm::Module &CurModule) {
  const llvm::Function *firstFunc = nullptr;
  ManualAnnotationSelection::Result res{nullptr};
  // if (F.isDeclaration()) {
  LLVM_DEBUG(llvm::dbgs() << "processing module: " << CurModule.getName()
                          << "\n";);
  // return false;
  if (CurModule.getFunctionList().size()) {
    firstFunc = &*CurModule.functions().begin();
  }
  //}
  if (firstFunc && firstFunc->isDeclaration()) {
    return res;
  }

  LLVM_DEBUG(llvm::dbgs() << "processing func: " << firstFunc->getName()
                          << "\n";);
  // LLVM_DEBUG(llvm::dbgs() << "processing func: " << F.getName() << '\n';);
  res.theInstruction = &*firstFunc->begin()->begin();

  return res;
}

} // namespace oft
