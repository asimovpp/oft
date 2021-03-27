#pragma once

#include "OverflowTool/Config.hpp"
#include "OverflowTool/ScaleGraph.hpp"

#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/MemorySSA.h"

#include <unordered_set>

namespace llvm {
class Module;
class Function;
class Value;
class GEPOperator;
class StoreInst;
} // namespace llvm

namespace oft {

struct ScaleVariableTracingInfo {
    const scale_graph scale_graph;
};

struct ScaleVariableTracing {
    std::map<Function *, MemorySSA *> mssas;

    using Result = ScaleVariableTracingInfo;

    scale_graph *createScaleGraph(std::vector<llvm::Value *> scale_variables);

    void
    traceScaleInstructionsUpToCalls(llvm::Value *V,
                                    std::unordered_set<llvm::Value *> &visited,
                                    scale_graph *sg);

    std::vector<llvm::Value *> traceCallInstruction(llvm::Value *V,
                                                    scale_graph *sg);

    std::vector<llvm::Instruction *> getUsingInstr(llvm::StoreInst *storeInst);

    void findGEPs(llvm::Value *V, std::vector<llvm::Value *> &geps);

    bool gepsAreEqual(llvm::GEPOperator *a, llvm::GEPOperator *b);

    Result perform(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);
};

} // namespace oft
