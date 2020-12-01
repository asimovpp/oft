
//
//
//

#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/IR/PassManager.h"
// using llvm::ModuleAnalysisManager
// using llvm::PassInfoMixin

namespace llvm {
class Module;
} // namespace llvm

#define OFT_OVERFLOWTRACKING_PASS_NAME "oft-overflow-tracking"

namespace oft {

class OverflowTrackingPass
    : public llvm::PassInfoMixin<OverflowTrackingPass> {
public:
  OverflowTrackingPass();

  llvm::PreservedAnalyses run(llvm::Module &CurModule,
                              llvm::ModuleAnalysisManager &MAM);
};

} // namespace oft
