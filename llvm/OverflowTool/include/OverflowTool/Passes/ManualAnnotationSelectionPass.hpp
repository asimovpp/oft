
//
//
//

#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/IR/PassManager.h"
// using llvm::ModuleAnalysisManager
// using llvm::PassInfoMixin

namespace llvm {
class Module;
} // namespace llvm

#define SDC_MANUALANNOTATIONSELECTOR_PASS_NAME "oft-manual-annotation-selection"

namespace ovt {

class ManualAnnotationSelectionPass
    : public llvm::PassInfoMixin<ManualAnnotationSelectionPass> {
public:
  ManualAnnotationSelectionPass();

  llvm::PreservedAnalyses run(llvm::Module &CurModule,
                              llvm::ModuleAnalysisManager &MAM);
};

} // namespace ovt
