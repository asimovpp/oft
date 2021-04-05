#include "OverflowTool/Analysis/Passes/ScaleOverflowIntegerDetectionPass.hpp"

#include "OverflowTool/Analysis/Passes/ScaleVariableTracingPass.hpp"
#include "OverflowTool/Analysis/ScaleOverflowIntegerDetection.hpp"
#include "OverflowTool/Config.hpp"

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/PassManager.h"
#include "llvm/Support/CommandLine.h"
// using llvm::cl::ParseEnvironmentOptions
// using llvm::cl::ResetAllOptionOccurrences

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
    auto graph = MAM.getResult<ScaleVariableTracingPass>(CurModule).graph;

    ScaleOverflowIntegerDetection detection;
    return detection.perform(CurModule, graph);
}

} // namespace oft
