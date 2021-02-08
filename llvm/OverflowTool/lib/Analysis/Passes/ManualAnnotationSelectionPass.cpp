
//
//

#include "OverflowTool/Config.hpp"

#include "OverflowTool/Analysis/Passes/ManualAnnotationSelectionPass.hpp"

#include "OverflowTool/Analysis/ManualAnnotationSelection.hpp"

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/Support/CommandLine.h"
// using llvm::cl::ParseEnvironmentOptions
// using llvm::cl::ResetAllOptionOccurrences

#include "llvm/Support/Debug.h"
// using LLVM_DEBUG macro
// using llvm::dbgs

#define DEBUG_TYPE OFT_MANUALANNOTATIONSELECTION_PASS_NAME
#define PASS_CMDLINE_OPTIONS_ENVVAR "MANUALANNOTATIONSELECTION_CMDLINE_OPTIONS"

llvm::AnalysisKey oft::ManualAnnotationSelectionPass::Key;

namespace oft {

// new passmanager pass

ManualAnnotationSelectionPass::ManualAnnotationSelectionPass() {
  llvm::cl::ResetAllOptionOccurrences();
  llvm::cl::ParseEnvironmentOptions(DEBUG_TYPE, PASS_CMDLINE_OPTIONS_ENVVAR);
}

ManualAnnotationSelectionPass::Result
ManualAnnotationSelectionPass::run(llvm::Module &CurModule,
                                   llvm::ModuleAnalysisManager &MAM) {
  ManualAnnotationSelection mas;
  mas.visit(CurModule);
  return mas.getAnnotated();
}

} // namespace oft
