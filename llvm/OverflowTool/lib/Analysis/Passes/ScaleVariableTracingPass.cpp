#include "OverflowTool/Config.hpp"

//#include "OverflowTool/Analysis/Passes/LibraryScaleVariableDetectionPass.hpp"

//#include "OverflowTool/Analysis/LibraryScaleVariableDetection.hpp"

#include "OverflowTool/Analysis/Passes/ScaleVariableTracingPass.hpp"
#include "OverflowTool/Analysis/ScaleVariableTracing.hpp"

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

#define DEBUG_TYPE OFT_SCALEVARTRACING_PASS_NAME
#define PASS_CMDLINE_OPTIONS_ENVVAR "SCALEVARTRACING_CMDLINE_OPTIONS"

llvm::AnalysisKey oft::ScaleVariableTracingPass::Key;

namespace oft {

// new passmanager pass

ScaleVariableTracingPass::ScaleVariableTracingPass() {
    llvm::cl::ResetAllOptionOccurrences();
    llvm::cl::ParseEnvironmentOptions(DEBUG_TYPE, PASS_CMDLINE_OPTIONS_ENVVAR);
}

ScaleVariableTracingPass::Result
ScaleVariableTracingPass::run(llvm::Module &CurModule,
                              llvm::ModuleAnalysisManager &MAM) {
    ScaleVariableTracing pass;
    return pass.perform(CurModule, MAM);
}

} // namespace oft
