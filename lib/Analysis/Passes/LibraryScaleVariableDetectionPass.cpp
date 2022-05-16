#include "OverflowTool/Analysis/Passes/LibraryScaleVariableDetectionPass.hpp"

#include "OverflowTool/Analysis/LibraryScaleVariableDetection.hpp"
#include "OverflowTool/Config.hpp"

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/PassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE OFT_LIBSCALEVARDETECTION_PASS_NAME

llvm::AnalysisKey oft::LibraryScaleVariableDetectionPass::Key;

namespace oft {

// new passmanager pass

LibraryScaleVariableDetectionPass::LibraryScaleVariableDetectionPass() {}

LibraryScaleVariableDetectionPass::Result
LibraryScaleVariableDetectionPass::run(llvm::Module &CurModule,
                                       llvm::ModuleAnalysisManager &MAM) {
    LibraryScaleVariableDetection pass;
    return pass.perform(CurModule, MAM);
}

} // namespace oft
