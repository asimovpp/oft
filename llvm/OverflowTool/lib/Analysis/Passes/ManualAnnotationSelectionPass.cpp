
//
//

#include "OverflowTool/Analysis/Passes/ManualAnnotationSelectionPass.hpp"

#include "OverflowTool/Analysis/ManualAnnotationSelection.hpp"
#include "OverflowTool/Config.hpp"

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/Support/CommandLine.h"
// using llvm::cl::ParseEnvironmentOptions
// using llvm::cl::ResetAllOptionOccurrences

#define DEBUG_TYPE OFT_MANUALANNOTATIONSELECTION_PASS_NAME

llvm::AnalysisKey oft::ManualAnnotationSelectionPass::Key;

namespace oft {

// new passmanager pass

ManualAnnotationSelectionPass::ManualAnnotationSelectionPass() {
}

ManualAnnotationSelectionPass::Result
ManualAnnotationSelectionPass::run(llvm::Module &CurModule,
                                   llvm::ModuleAnalysisManager &MAM) {
    ManualAnnotationSelection mas;
    mas.visit(CurModule);
    return mas.getAnnotated();
}

llvm::PreservedAnalyses
ManualAnnotationSelectionPrinterPass::run(llvm::Module &M,
                                          llvm::ModuleAnalysisManager &AM) {
    ManualAnnotationSelectionPass::Result &res =
        AM.getResult<ManualAnnotationSelectionPass>(M);

    OS << "Manual annotations for module: " << M.getName() << "\n";
    for (const auto &e : res.values) {
        OS << *e << "\n";
    }

    return llvm::PreservedAnalyses::all();
}

} // namespace oft
