
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

#define SDC_MANUALANNOTATIONSELECTOR_PASS_NAME "oft-manual-annotation-selector"

namespace ovt {

class ManualAnnotationSelectorPass
    : public llvm::PassInfoMixin<ManualAnnotationSelectorPass> {
public:
  ManualAnnotationSelectorPass();

  llvm::PreservedAnalyses run(llvm::Module &CurModule,
                              llvm::ModuleAnalysisManager &MAM);
};

} // namespace ovt
