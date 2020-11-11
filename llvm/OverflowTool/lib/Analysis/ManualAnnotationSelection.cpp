//
//
//

#include "OverflowTool/ManualAnnotationSelection.hpp"

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/Debug.h"
// using LLVM_DEBUG macro
// using llvm::dbgs

#define DEBUG_TYPE "oft-manual-selection"

namespace oft {

bool ManualAnnotationSelection::perform(const llvm::Module &CurModule) {
  // if (F.isDeclaration()) {
  // return false;
  //}

  // LLVM_DEBUG(llvm::dbgs() << "processing func: " << F.getName() << '\n';);

  return false;
}

} // namespace oft
