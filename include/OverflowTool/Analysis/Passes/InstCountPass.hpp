#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

#define OFT_INSTCOUNT_PASS_NAME "oft-instcount"

namespace llvm {
class Module;
}

namespace oft {

struct InstCountPass : llvm::PassInfoMixin<InstCountPass> {
    llvm::PreservedAnalyses run(llvm::Module &F, llvm::ModuleAnalysisManager &);
};

} // namespace oft
