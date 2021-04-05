#pragma once

#include "OverflowTool/Config.hpp"

#include <unordered_set>

namespace llvm {
class Module;
class Value;
class Instruction;
} // namespace llvm

namespace oft {

class scale_graph;
class scale_node;

struct ScaleOverflowIntegerDetectionInfo {
    std::unordered_set<llvm::Instruction *> overflowable;
};

struct ScaleOverflowIntegerDetection {
    using Result = ScaleOverflowIntegerDetectionInfo;
    template <typename T> using SetTy = std::unordered_set<T>;

    bool canIntegerOverflow(llvm::Value *V);

    void findInstructions(scale_node *node,
                          SetTy<llvm::Instruction *> *overflowable,
                          SetTy<scale_node *> &visited);

    Result perform(llvm::Module &M, scale_graph &Graph);
};

} // namespace oft
