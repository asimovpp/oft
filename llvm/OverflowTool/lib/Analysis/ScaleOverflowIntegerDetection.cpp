#include "OverflowTool/Analysis/ScaleOverflowIntegerDetection.hpp"

#include "OverflowTool/Debug.hpp"
#include "OverflowTool/ScaleGraph.hpp"
#include "OverflowTool/UtilFuncs.hpp"

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "oft-scale-overflow-int-detection"

using namespace llvm;

namespace oft {

/*
Check that it is the right type of instruction and
that it is one of the instructions we care about
i.e. an arithmetic function operating on an integer 32 bits in size or smaller.
*/
bool ScaleOverflowIntegerDetection::canIntegerOverflow(Value *V) {
    const std::unordered_set<unsigned> overflow_ops = {
        Instruction::Add, Instruction::Sub,  Instruction::Mul,
        Instruction::Shl, Instruction::LShr, Instruction::AShr};
    // TODO: what would happen if the operation was between 32 bit and 64 bit
    // values? would the needed cast be in a separate instrucion somewhere?
    if (BinaryOperator *I = dyn_cast<BinaryOperator>(V)) {
        if (overflow_ops.find(I->getOpcode()) != overflow_ops.end() &&
            I->getType()->isIntegerTy() &&
            I->getType()->getScalarSizeInBits() <= 32) {
            OFT_DEBUG(dbgs() << "     Instruction " << *I
                             << " could overflow. Has type " << *(I->getType())
                             << "\n";);
            return true;
        }
    }
    return false;
}

/*
Traverse scale graph starting from "node", tag instructions that can overflow
and add them to list of to-be-instrumented-instructions.
*/
void ScaleOverflowIntegerDetection::findInstructions(
    scale_node *node,
    std::unordered_set<Instruction *> *overflowable,
    std::unordered_set<scale_node *> &visited) {
    if (visited.find(node) != visited.end())
        return;

    visited.insert(node);
    // check each visited node whether it should be instrumented and add to a
    // list if it should be
    if (canIntegerOverflow(node->value)) {
        overflowable->insert(cast<Instruction>(node->value));
        node->could_overflow = true;
    }

    for (scale_node *n : node->children)
        findInstructions(n, overflowable, visited);
}

ScaleOverflowIntegerDetection::Result
ScaleOverflowIntegerDetection::perform(Module &M, scale_graph &Graph) {
    std::unordered_set<Instruction *> overflowable;

    for (scale_node *v : Graph.scale_vars) {
        std::unordered_set<scale_node *> visited;
        findInstructions(v, &overflowable, visited);
    }

    ScaleOverflowIntegerDetection::Result res{overflowable};

    return res;
}

} // namespace oft
