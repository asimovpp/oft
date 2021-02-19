#pragma once

#include "OverflowTool/Config.hpp"
#include "OverflowTool/ScaleGraph.hpp"

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

#include <unordered_set>

namespace oft {

struct ScaleOverflowIntegerDetectionInfo {
    const std::unordered_set<Instruction *> overflowable_int_instructions;
};

struct ScaleOverflowIntegerDetection {
    using Result = ScaleOverflowIntegerDetectionInfo;

    bool canIntegerOverflow(llvm::Value *V);

    void findInstructions(
        scale_node *node,
        std::unordered_set<llvm::Instruction *> *overflowable_int_instructions,
        std::unordered_set<scale_node *> &visited);

    Result perform(llvm::Module &M, llvm::ModuleAnalysisManager &AM);
};

} // namespace oft
