
//
//
//

#pragma once

#include "OverflowTool/Config.hpp"

#include "OverflowTool/ManualAnnotationSelector.hpp"

#include "llvm/IR/PassManager.h"
// using llvm::FunctionAnalysisManager
// using llvm::PassInfoMixin

namespace llvm {
class Function;
} // namespace llvm

#define SDC_MANUALANNOTATIONSELECTOR_PASS_NAME "oft-manual-annotation-selector"

namespace sdc {

class ManualAnnotationSelectorPass
    : public llvm::PassInfoMixin<ManualAnnotationSelectorPass> {
public:
  ManualAnnotationSelectorPass();

  bool perform(llvm::Function &F);

  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &FAM);
};

} // namespace sdc
