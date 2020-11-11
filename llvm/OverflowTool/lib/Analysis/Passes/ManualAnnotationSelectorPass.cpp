
//
//

#include "OverflowTool/Config.hpp"

#include "OverflowTool/Passes/ManualAnnotationSelectorPass.hpp"

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

#define DEBUG_TYPE SDC_MANUALANNOTATIONSELECTOR_PASS_NAME
#define PASS_CMDLINE_OPTIONS_ENVVAR "MANUALANNOTATIONSELECTION_CMDLINE_OPTIONS"

namespace sdc {

// new passmanager pass

ManualAnnotationSelectorPass::ManualAnnotationSelectorPass() {
  llvm::cl::ResetAllOptionOccurrences();
  llvm::cl::ParseEnvironmentOptions(DEBUG_TYPE, PASS_CMDLINE_OPTIONS_ENVVAR);
}

bool ManualAnnotationSelectorPass::perform(llvm::Function &F) {
  auto not_in = [](const auto &C, const auto &E) {
    return C.end() == std::find(std::begin(C), std::end(C), E);
  };

  if (F.isDeclaration()) {
    return false;
  }

  LLVM_DEBUG(llvm::dbgs() << "processing func: " << F.getName() << '\n';);

  bool hasChanged = false;

  return hasChanged;
}

llvm::PreservedAnalyses
ManualAnnotationSelectorPass::run(llvm::Function &F,
                                  llvm::FunctionAnalysisManager &FAM) {
  perform(F);

  return llvm::PreservedAnalyses::all();
}

} // namespace sdc
