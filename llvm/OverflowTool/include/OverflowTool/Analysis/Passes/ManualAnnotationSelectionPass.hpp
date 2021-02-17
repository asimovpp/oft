//
//
//

#pragma once

#include "OverflowTool/Config.hpp"

#include "OverflowTool/Analysis/ManualAnnotationSelection.hpp"

#include "llvm/IR/PassManager.h"
// using llvm::ModuleAnalysisManager
// using llvm::AnalysisInfoMixin
// using llvm::PassInfoMixin

#include "llvm/Support/Debug.h"
// using LLVM_DEBUG macro
// using llvm::raw_ostream

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

  Result run(llvm::Module &CurModule, llvm::ModuleAnalysisManager &MAM);
};

// Printer pass for ManualAnnotationSelectionPass results
class ManualAnnotationSelectionPrinterPass
    : public llvm::PassInfoMixin<ManualAnnotationSelectionPrinterPass> {
  llvm::raw_ostream &OS;

public:
  explicit ManualAnnotationSelectionPrinterPass(llvm::raw_ostream &OS)
      : OS(OS) {}

  llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &AM);
};

} // namespace oft
