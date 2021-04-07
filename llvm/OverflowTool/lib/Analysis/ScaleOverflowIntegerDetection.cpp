#include "OverflowTool/Analysis/ScaleOverflowIntegerDetection.hpp"

#include "OverflowTool/Debug.hpp"
#include "OverflowTool/ScaleGraph.hpp"
#include "OverflowTool/UtilFuncs.hpp"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "oft-scale-overflow-int-detection-analysis"

STATISTIC(NumOverflowableIntegerInstructions,
          "Number of 32-bit integer instructions that can overflow");

using namespace llvm;

namespace oft {

/*
Check that operation is the right type and one of the instructions we care
about, i.e., an arithmetic operation on an 32-bit integer or smaller.
*/
bool ScaleOverflowIntegerDetection::canIntegerOverflow(Value *V) {
    // TODO: what would happen if the operation was between 32 bit and 64 bit
    // values? would the needed cast be in a separate instrucion somewhere?
    if (BinaryOperator *I = dyn_cast<BinaryOperator>(V)) {
        if (OverflowOps.count(I->getOpcode()) && I->getType()->isIntegerTy() &&
            I->getType()->getScalarSizeInBits() <= OverflowBitsThreshold) {
            OFT_DEBUG(dbgs() << "overflowable instruction " << *I << " of type "
                             << *I->getType() << "\n";);

            return true;
        }
    }

    return false;
}

/*
Traverse scale graph starting from "node", and add nodes to list if they can
overflow
*/
void ScaleOverflowIntegerDetection::findInstructions(
    const scale_node &node, SetTy<scale_node *> &overflowable_nodes,
    SetTy<const scale_node *> &visited) {
    if (visited.find(&node) != visited.end())
        return;

    visited.insert(&node);

    // add to list nodes that should be instrumented
    if (canIntegerOverflow(node.value)) {
        overflowable_nodes.insert(const_cast<scale_node *>(&node));
    }

    for (scale_node *n : node.children)
        findInstructions(*n, overflowable_nodes, visited);
}

/*
Perform integer overflow detection analysis
*/
ScaleOverflowIntegerDetection::Result
ScaleOverflowIntegerDetection::perform(const Module &M, scale_graph &Graph) {
    SetTy<scale_node *> overflowable_nodes;

    for (scale_node *v : Graph.scale_vars) {
        SetTy<const scale_node *> visited;
        findInstructions(*v, overflowable_nodes, visited);
    }

    SetTy<Instruction *> overflowable;
    for (auto &e : overflowable_nodes) {
        e->could_overflow = true;
        overflowable.insert(cast<Instruction>(e->value));
    }

    NumOverflowableIntegerInstructions += overflowable.size();

    return {{std::begin(overflowable), std::end(overflowable)}, Graph};
}

} // namespace oft
