#include "OverflowTool/Analysis/Passes/LibraryScaleVariableDetectionPass.hpp"

#include "OverflowTool/Analysis/LibraryScaleVariableDetection.hpp"
#include "OverflowTool/Config.hpp"

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/PassManager.h"
#include "llvm/Support/CommandLine.h"
// using llvm::cl::ParseEnvironmentOptions
// using llvm::cl::ResetAllOptionOccurrences

#define DEBUG_TYPE OFT_LIBSCALEVARDETECTION_PASS_NAME
#define PASS_CMDLINE_OPTIONS_ENVVAR "LIBSCALEVARDETECTION_CMDLINE_OPTIONS"

llvm::AnalysisKey oft::LibraryScaleVariableDetectionPass::Key;

namespace oft {

// new passmanager pass

LibraryScaleVariableDetectionPass::LibraryScaleVariableDetectionPass() {
    llvm::cl::ResetAllOptionOccurrences();
    llvm::cl::ParseEnvironmentOptions(DEBUG_TYPE, PASS_CMDLINE_OPTIONS_ENVVAR);
}

LibraryScaleVariableDetectionPass::Result
LibraryScaleVariableDetectionPass::run(llvm::Module &CurModule,
                                       llvm::ModuleAnalysisManager &MAM) {
    LibraryScaleVariableDetection pass;
    return pass.perform(CurModule, MAM);
}

} // namespace oft
