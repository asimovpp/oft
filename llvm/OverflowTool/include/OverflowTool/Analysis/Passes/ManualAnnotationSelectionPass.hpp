//

#pragma once

#include "OverflowTool/Analysis/ManualAnnotationSelection.hpp"
#include "OverflowTool/Config.hpp"

#include "llvm/IR/PassManager.h"
#include "llvm/Support/Debug.h"

#define OFT_MANUALANNOTATIONSELECTION_PASS_NAME                                \
    "oft-manual-annotation-selection"

namespace llvm {
class Module;
} // namespace llvm

namespace oft {

class ManualAnnotationSelectionPass
    : public llvm::AnalysisInfoMixin<ManualAnnotationSelectionPass> {
    friend llvm::AnalysisInfoMixin<ManualAnnotationSelectionPass>;
    static llvm::AnalysisKey Key;
    annotation_db_t ParsedAnnotations;
    std::vector<std::string> normAnnotationFiles;

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

    llvm::PreservedAnalyses run(llvm::Module &M,
                                llvm::ModuleAnalysisManager &AM);
};

} // namespace oft
