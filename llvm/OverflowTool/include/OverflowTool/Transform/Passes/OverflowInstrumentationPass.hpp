
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

#define OFT_OVERFLOWINSTRUMENTATION_PASS_NAME "oft-overflow-instrumentation"

namespace oft {

class OverflowInstrumentationPass
    : public llvm::PassInfoMixin<OverflowInstrumentationPass> {
  public:
    OverflowInstrumentationPass();

    llvm::PreservedAnalyses run(llvm::Module &CurModule,
                                llvm::ModuleAnalysisManager &MAM);
};

} // namespace oft
