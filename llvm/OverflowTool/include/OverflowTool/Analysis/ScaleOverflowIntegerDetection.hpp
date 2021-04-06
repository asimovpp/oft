#pragma once

#include "OverflowTool/Config.hpp"

#include <set>
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

    const unsigned int OverflowBitsThreshold = 32;

    template <typename T> ScaleOverflowIntegerDetection(T begin, T end) {
        OverflowOps.insert(begin, end);
    }

    bool canIntegerOverflow(llvm::Value *V);

    void findInstructions(const scale_node &node,
                          SetTy<scale_node *> &overflowable_nodes,
                          SetTy<const scale_node *> &visited);

    Result perform(llvm::Module &M, scale_graph &Graph);

  private:
    std::set<unsigned> OverflowOps;
};

} // namespace oft
