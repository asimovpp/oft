//
//
//

#pragma once

#include "OverflowTool/Config.hpp"

#include "OverflowTool/Analysis/ManualAnnotationSelection.hpp"

#include "llvm/IR/PassManager.h"
// using llvm::ModuleAnalysisManager
// using llvm::PassInfoMixin

namespace llvm {
class Module;
} // namespace llvm

#define OFT_MANUALANNOTATIONSELECTION_PASS_NAME                                \
  "oft-manual-annotation-selection"

namespace oft {

class ManualAnnotationSelectionPass
    : public llvm::AnalysisInfoMixin<ManualAnnotationSelectionPass> {
  friend llvm::AnalysisInfoMixin<ManualAnnotationSelectionPass>;
  static llvm::AnalysisKey Key;

public:
  using Result = ManualAnnotationSelection::Result;

  ManualAnnotationSelectionPass();

  Result run(llvm::Module &CurModule,
                              llvm::ModuleAnalysisManager &MAM);
};

} // namespace oft
