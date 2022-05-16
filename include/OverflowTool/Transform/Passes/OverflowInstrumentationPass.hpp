#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/IR/PassManager.h"

namespace llvm {
class Module;
} // namespace llvm

#define OFT_OVERFLOWINSTRUMENTATION_PASS_NAME "oft-overflow-instrumentation"

namespace oft {

class OverflowInstrumentationPass
    : public llvm::PassInfoMixin<OverflowInstrumentationPass> {
  public:
    OverflowInstrumentationPass();

    llvm::PreservedAnalyses run(llvm::Module &M,
                                llvm::ModuleAnalysisManager &MAM);
};

} // namespace oft
