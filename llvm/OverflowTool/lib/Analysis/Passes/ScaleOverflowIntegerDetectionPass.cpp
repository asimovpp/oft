#include "OverflowTool/Config.hpp"

#include "OverflowTracking/Analysis/Passes/ScaleOverflowIntegerDetectionPass.hpp"

#include "OverflowTracking/Analysis/ScaleOverflowIntegerDetection.hpp"

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/PassManager.h"
#include "llvm/Support/CommandLine.h"
// using llvm::cl::ParseEnvironmentOptions
// using llvm::cl::ResetAllOptionOccurrences

#include "llvm/Support/Debug.h"
// using LLVM_DEBUG macro
// using llvm::dbgs

#define DEBUG_TYPE OFT_SCALEOVERFLOWINTDET_PASS_NAME
#define PASS_CMDLINE_OPTIONS_ENVVAR "SCALEOVERFLOWINTDET_CMDLINE_OPTIONS"

llvm::AnalysisKey oft::ScaleOverflowIntegerDetectionPass::Key;

namespace oft {

// new passmanager pass

ScaleOverflowIntegerDetectionPass::ScaleOverflowIntegerDetectionPass() {
  llvm::cl::ResetAllOptionOccurrences();
  llvm::cl::ParseEnvironmentOptions(DEBUG_TYPE, PASS_CMDLINE_OPTIONS_ENVVAR);
}

ScaleOverflowIntegerDetectionPass::Result
ScaleOverflowIntegerDetectionPass::run(llvm::Module &CurModule,
                                       llvm::ModuleAnalysisManager &MAM) {
  ScaleOverflowIntegerDetection pass;
  return pass.perform(CurModule, MAM);
}

} // namespace oft
