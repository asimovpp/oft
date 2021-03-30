
//
//

#include "OverflowTool/Transform/Passes/OverflowInstrumentationPass.hpp"

#include "OverflowTool/Config.hpp"
#include "OverflowTool/Transform/OverflowInstrumentation.hpp"

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/Support/CommandLine.h"
// using llvm::cl::ParseEnvironmentOptions
// using llvm::cl::ResetAllOptionOccurrences

#define DEBUG_TYPE OFT_OVERFLOWINSTRUMENTATION_PASS_NAME
#define PASS_CMDLINE_OPTIONS_ENVVAR "OVERFLOWINSTRUMENTATION_CMDLINE_OPTIONS"

namespace oft {

// new passmanager pass

OverflowInstrumentationPass::OverflowInstrumentationPass() {
    llvm::cl::ResetAllOptionOccurrences();
    llvm::cl::ParseEnvironmentOptions(DEBUG_TYPE, PASS_CMDLINE_OPTIONS_ENVVAR);
}

llvm::PreservedAnalyses
OverflowInstrumentationPass::run(llvm::Module &CurModule,
                                 llvm::ModuleAnalysisManager &MAM) {

    OverflowInstrumentation pass;
    pass.perform(CurModule, MAM);

    return llvm::PreservedAnalyses::none();
}

} // namespace oft
