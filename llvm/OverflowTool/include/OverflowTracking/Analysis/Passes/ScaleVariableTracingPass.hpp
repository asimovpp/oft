#pragma once

#include "OverflowTool/Config.hpp"
#include "OverflowTracking/Analysis/ScaleVariableTracing.hpp"

#include "llvm/IR/PassManager.h"

namespace llvm {
class Module;
} // namespace llvm

#define OFT_SCALEVARTRACING_PASS_NAME "oft-scale-var-tracing"

namespace oft {

class ScaleVariableTracingPass
    : public llvm::AnalysisInfoMixin<ScaleVariableTracingPass> {
    friend llvm::AnalysisInfoMixin<ScaleVariableTracingPass>;
    static llvm::AnalysisKey Key;

public:
  using Result = ScaleVariableTracing::Result;

  ScaleVariableTracingPass();

  Result run(llvm::Module &CurModule,
                              llvm::ModuleAnalysisManager &MAM);
};

} // namespace oft
