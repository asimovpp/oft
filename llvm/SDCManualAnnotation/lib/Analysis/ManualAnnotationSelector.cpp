//
//
//

#include "SDCManualAnnotation/Analysis/ManualAnnotationSelector.hpp"

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/Debug.h"
// using LLVM_DEBUG macro
// using llvm::dbgs

#define DEBUG_TYPE "sdc-manual-selector"

namespace sdc {

ManualAnnotationSelector::ManualAnnotationSelector(
    const llvm::Function &CurFunc) {}

} // namespace sdc

