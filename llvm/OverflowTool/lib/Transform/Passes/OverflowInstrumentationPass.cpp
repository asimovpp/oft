#include "OverflowTool/Transform/Passes/OverflowInstrumentationPass.hpp"

#include "OverflowTool/Analysis/Passes/ScaleOverflowIntegerDetectionPass.hpp"
#include "OverflowTool/Config.hpp"
#include "OverflowTool/Transform/OverflowInstrumentation.hpp"

#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE OFT_OVERFLOWINSTRUMENTATION_PASS_NAME
#define PASS_CMDLINE_OPTIONS_ENVVAR "OVERFLOWINSTRUMENTATION_CMDLINE_OPTIONS"

namespace oft {

// new passmanager pass

OverflowInstrumentationPass::OverflowInstrumentationPass() {
    llvm::cl::ResetAllOptionOccurrences();
    llvm::cl::ParseEnvironmentOptions(DEBUG_TYPE, PASS_CMDLINE_OPTIONS_ENVVAR);
}

llvm::PreservedAnalyses
OverflowInstrumentationPass::run(llvm::Module &M,
                                 llvm::ModuleAnalysisManager &MAM) {
    const auto res = MAM.getResult<ScaleOverflowIntegerDetectionPass>(M);

    OverflowInstrumentation oi(M);
    oi.instrument(res.overflowable);

    res.graph.print(llvm::dbgs());

    return llvm::PreservedAnalyses::none();
}

} // namespace oft
