#pragma once

#include "OverflowTool/Analysis/LibraryScaleVariableDetection.hpp"
#include "OverflowTool/Config.hpp"

#include "llvm/IR/PassManager.h"

namespace llvm {
class Module;
} // namespace llvm

#define OFT_LIBSCALEVARDETECTION_PASS_NAME "oft-library-scale-vars"

namespace oft {

class LibraryScaleVariableDetectionPass
    : public llvm::AnalysisInfoMixin<LibraryScaleVariableDetectionPass> {
    friend llvm::AnalysisInfoMixin<LibraryScaleVariableDetectionPass>;
    static llvm::AnalysisKey Key;

  public:
    using Result = LibraryScaleVariableDetection::Result;

    LibraryScaleVariableDetectionPass();

    Result run(llvm::Module &CurModule, llvm::ModuleAnalysisManager &MAM);
};

} // namespace oft
