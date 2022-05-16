#pragma once

#include "OverflowTool/Analysis/ScaleVariableTracing.hpp"
#include "OverflowTool/Config.hpp"

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

    Result run(llvm::Module &CurModule, llvm::ModuleAnalysisManager &MAM);
};

// printer pass for ScaleVariableTracing results
class ScaleVariableTracingPrinterPass
    : public llvm::PassInfoMixin<ScaleVariableTracingPrinterPass> {
    llvm::raw_ostream &OS;

  public:
    explicit ScaleVariableTracingPrinterPass(llvm::raw_ostream &OS) : OS(OS) {}

    llvm::PreservedAnalyses run(llvm::Module &M,
                                llvm::ModuleAnalysisManager &AM);
};

} // namespace oft
