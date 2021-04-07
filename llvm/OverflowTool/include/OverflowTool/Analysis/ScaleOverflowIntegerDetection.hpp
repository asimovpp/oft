#pragma once

#include "OverflowTool/Config.hpp"
#include "OverflowTool/ScaleGraph.hpp"

#include <set>
#include <vector>

namespace llvm {
class Module;
class Value;
class Instruction;
} // namespace llvm

namespace oft {

struct ScaleOverflowIntegerDetectionInfo {
    std::vector<llvm::Instruction *> overflowable;
    scale_graph graph;
};

struct ScaleOverflowIntegerDetection {
    using Result = ScaleOverflowIntegerDetectionInfo;
    template <typename T> using SetTy = std::set<T>;

    const unsigned int OverflowBitsThreshold = 32;

    template <typename T> ScaleOverflowIntegerDetection(T begin, T end) {
        OverflowOps.insert(begin, end);
    }

    bool canIntegerOverflow(llvm::Value *V);

    void findInstructions(const scale_node &node,
                          SetTy<scale_node *> &overflowable_nodes,
                          SetTy<const scale_node *> &visited);

    Result perform(const llvm::Module &M, scale_graph &Graph);

  private:
    SetTy<unsigned> OverflowOps;
};

} // namespace oft
