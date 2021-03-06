#pragma once

#include "OverflowTool/Analysis/ScaleOverflowIntegerDetection.hpp"
#include "OverflowTool/Config.hpp"

#include "llvm/IR/PassManager.h"

namespace llvm {
class Module;
} // namespace llvm

#define OFT_SCALEOVERFLOWINTDET_PASS_NAME "oft-scale-overflow-int-det"

namespace oft {

class ScaleOverflowIntegerDetectionPass
    : public llvm::AnalysisInfoMixin<ScaleOverflowIntegerDetectionPass> {
    friend llvm::AnalysisInfoMixin<ScaleOverflowIntegerDetectionPass>;
    static llvm::AnalysisKey Key;

  public:
    using Result = ScaleOverflowIntegerDetection::Result;

    ScaleOverflowIntegerDetectionPass();

    Result run(llvm::Module &CurModule, llvm::ModuleAnalysisManager &MAM);
};

} // namespace oft
