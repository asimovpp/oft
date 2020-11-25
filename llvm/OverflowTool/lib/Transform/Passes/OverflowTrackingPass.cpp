
//
//

#include "OverflowTool/Config.hpp"

#include "OverflowTracking/Transform/Passes/OverflowTrackingPass.hpp"

#include "OverflowTracking/Transform/OverflowTracking.hpp"

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

#define DEBUG_TYPE OFT_OVERFLOWTRACKING_PASS_NAME
#define PASS_CMDLINE_OPTIONS_ENVVAR "OVERFLOWTRACKING_CMDLINE_OPTIONS"

namespace oft {

// new passmanager pass

OverflowTrackingPass::OverflowTrackingPass() {
  llvm::cl::ResetAllOptionOccurrences();
  llvm::cl::ParseEnvironmentOptions(DEBUG_TYPE, PASS_CMDLINE_OPTIONS_ENVVAR);
}

llvm::PreservedAnalyses
OverflowTrackingPass::run(llvm::Module &CurModule,
                          llvm::ModuleAnalysisManager &MAM) {
  auto &FAM = MAM.getResult<llvm::FunctionAnalysisManagerModuleProxy>(CurModule).getManager();

  AnalyseScale mas;
  for(auto &F : CurModule) {
    if(F.isDeclaration()) {
      continue;
    }

    auto &MSSA = FAM.getResult<llvm::MemorySSAAnalysis>(F).getMSSA();
    mas.mssas.insert(std::pair<Function*, MemorySSA*>(&F, &MSSA));    
  }
  mas.track(CurModule, MAM);

  return llvm::PreservedAnalyses::none();
}

} // namespace oft
