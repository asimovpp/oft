#pragma once

#include "OverflowTool/Config.hpp"
#include "OverflowTool/ScaleGraph.hpp"
#include "OverflowTool/UtilFuncs.hpp"

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

#include <unordered_set>

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
