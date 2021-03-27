#include "OverflowTool/Analysis/ScaleVariableTracing.hpp"

#include "OverflowTool/Analysis/Passes/LibraryScaleVariableDetectionPass.hpp"
#include "OverflowTool/Analysis/Passes/ManualAnnotationSelectionPass.hpp"
#include "OverflowTool/ScaleGraph.hpp"
#include "OverflowTool/UtilFuncs.hpp"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <unordered_set>

#define DEBUG_TYPE "oft-scale-var-tracing"

using namespace llvm;

namespace oft {
/*
Create a scale graph based on the provided list of scale variables (starting
points to tracing).
*/
scale_graph *
ScaleVariableTracing::createScaleGraph(std::vector<Value *> scale_variables) {
    scale_graph *sg = new scale_graph;

    for (Value *scale_var : scale_variables)
        sg->addvertex(scale_var, true);

    // Iterate through scale variables and find all instructions which they
    // influence (scale instructions)
    for (Value *V : scale_variables) {
        std::unordered_set<Value *> visited;
        if (isa<AllocaInst>(V)) {
            LLVM_DEBUG(dbgs() << "tracing scale var (alloca): " << *V << "\n";);
            traceScaleInstructionsUpToCalls(V, visited, sg);
        } else if (isa<GlobalVariable>(V)) {
            LLVM_DEBUG(dbgs() << "tracing scale var (global): " << *V << "\n";);
            traceScaleInstructionsUpToCalls(V, visited, sg);
        } else if (auto *gep = dyn_cast<GEPOperator>(V)) {
            LLVM_DEBUG(dbgs() << "tracing scale var (gep): " << *V << "\n";);
            std::vector<Value *> geps;
            auto *gepOp = gep->getPointerOperand()->stripPointerCasts();
            if (GlobalVariable *gv = dyn_cast<GlobalVariable>(gepOp)) {
                LLVM_DEBUG(dbgs() << "    it points to a global var\n";);
                findGEPs(gv, geps);
            } else if (auto *allocaV = dyn_cast<AllocaInst>(gepOp)) {
                LLVM_DEBUG(dbgs()
                               << "    it points to alloca " << *allocaV
                               << " with type "
                               << *(allocaV->getAllocatedType()) << " and size "
                               << *(allocaV->getArraySize()) << "\n";);

                if (auto *arrayV =
                        dyn_cast<ArrayType>(allocaV->getAllocatedType())) {
                    LLVM_DEBUG(dbgs() << "    And it is an array with size "
                                      << arrayV->getNumElements() << "\n";);
                    findGEPs(allocaV, geps);
                } else if (auto *structV = dyn_cast<StructType>(
                               allocaV->getAllocatedType())) {
                    LLVM_DEBUG(dbgs() << "    And it is an struct " << *structV
                                      << "\n";);
                    findGEPs(allocaV, geps);
                } else {
                    LLVM_DEBUG(dbgs() << "   And... cannot trace further\n";);
                }
            } else {
                LLVM_DEBUG(dbgs() << "No rule for tracing what " << *gepOp
                                  << " is pointing to\n";);
                printValue(V, 4);
            }

            LLVM_DEBUG(dbgs() << "    Other uses are in: \n";);
            for (auto *GVgep : geps) {
                if (gepsAreEqual(cast<GEPOperator>(gep),
                                 cast<GEPOperator>(GVgep))) {
                    // need to connect the equivalent gep (UUgep) to the
                    // original scale variable (gep) in order to make the
                    // scale graph sensible.
                    // TODO: Is there a better way of doing this?
                    sg->addvertex(GVgep, false);
                    sg->addedge(gep, GVgep);
                    traceScaleInstructionsUpToCalls(GVgep, visited, sg);
                }
            }

        } else {
            LLVM_DEBUG(dbgs() << "No rule for tracing value " << *V << "\n";);
        }
    }

    LLVM_DEBUG(dbgs() << "tracing call sites\n";);
    // expand call instructions iteratively until no more changes occur
    unsigned int prevSize = 0;
    while (sg->get_size() - prevSize != 0) {
        std::vector<Value *> to_follow;
        for (auto &node : sg->graph) {
            if (isa<CallInst>(node.first)) {
                LLVM_DEBUG(dbgs()
                               << "found call site " << *(node.first) << "\n";);
                std::vector<Value *> continuations =
                    traceCallInstruction(node.first, sg);
                to_follow.insert(to_follow.end(), continuations.begin(),
                                 continuations.end());
            }
        }

        for (Value *child : to_follow) {
            LLVM_DEBUG(dbgs()
                           << "following call site via " << *(child) << "\n";);
            std::unordered_set<Value *> visited;
            traceScaleInstructionsUpToCalls(child, visited, sg);
        }

        prevSize = sg->get_size();
    }

    return sg;
}

/*
Connect a call site to the called function's body via argument positions in the
scale graph.
*/
std::vector<Value *>
ScaleVariableTracing::traceCallInstruction(Value *V, scale_graph *sg) {
    std::vector<Value *> children;

    // the tracing is continued across function calls through argument position
    if (CallInst *callInst = dyn_cast<CallInst>(
            V)) { // TODO: also check if the number of users is =0?
        for (scale_node *parent : sg->getvertex(V)->parents) {
            // LLVM_DEBUG(dbgs() << "checking " << *callInst << " and user " <<
            // *V << "\n";);
            for (unsigned int i = 0; i < callInst->getNumOperands(); ++i) {
                // LLVM_DEBUG(dbgs() << "checking " << i << "th operand which is
                // " <<
                // *(callInst->getOperand(i)) << "\n";);
                if (callInst->getOperand(i) == parent->value) {
                    Function *fp = callInst->getCalledFunction();
                    if (fp == NULL)
                        fp = dyn_cast<Function>(
                            callInst->getCalledValue()->stripPointerCasts());
                    // LLVM_DEBUG(dbgs() << "V is " << i << "th operand of " <<
                    // *callInst
                    // << "; Function is " << fp->getName() << "\n";);
                    if (fp->isDeclaration()) {
                        // LLVM_DEBUG(dbgs() << "     Function body not
                        // available for further tracing. ( " << fp->getName()
                        // << " )\n";);
                    } else if (fp->isVarArg()) {
                        LLVM_DEBUG(
                            dbgs()
                                << "     Function is variadic. Don't know how "
                                   "to trace: "
                                << fp->getName() << "\n";);
                    } else {
                        // LLVM_DEBUG(dbgs() << "     Tracing in function body
                        // of called function via " << i << "th argument. ( " <<
                        // fp->getName()  << " )\n"; followChain(fp->getArg(i),
                        // depth+1, visited););
                        Value *arg_to_track = &*(fp->arg_begin() + i);
                        children.push_back(arg_to_track);
                        sg->addvertex(arg_to_track, false);
                        sg->addedge(V, arg_to_track);
                    }
                }
            }
        }
    }

    return children;
}

/*
Trace scale variable (arg 1) and add visited nodes to scale graph.
Stop at call sites.
Already visited nodes are skipped the second time.
*/
void ScaleVariableTracing::traceScaleInstructionsUpToCalls(
    Value *V, std::unordered_set<Value *> &visited, scale_graph *sg) {
    LLVM_DEBUG(dbgs() << "Visiting node " << visited.size() << "\t" << *V
                      << "\n";);
    if (visited.find(V) != visited.end()) {
        LLVM_DEBUG(dbgs() << "Node " << *V
                          << " has already been visited. Skipping.\n";);
        return;
    }
    visited.insert(V);

    std::vector<Value *> children;
    for (User *U : V->users()) {
        children.push_back(U);
        sg->addvertex(U, false);
        sg->addedge(V, U);
    }

    // store instructions require MemSSA to connect them to their corresponding
    // load instructions in the chain
    if (StoreInst *storeInst = dyn_cast<StoreInst>(
            V)) { // TODO: also check if the number of users is =0?
        std::vector<Instruction *> memUses = getUsingInstr(storeInst);
        children.insert(children.end(), memUses.begin(), memUses.end());
        for (Instruction *I : memUses) {
            children.push_back(I);
            sg->addvertex(I, false);
            sg->addedge(V, I);
        }
    }

    for (Value *child : children) {
        traceScaleInstructionsUpToCalls(child, visited, sg);
    }
}

/*
Use MemSSA to find load instructions corresponding to a store instruction.
*/
std::vector<Instruction *>
ScaleVariableTracing::getUsingInstr(StoreInst *storeInst) {
    std::vector<Instruction *> out;

    Function *caller = storeInst->getParent()->getParent();
    LLVM_DEBUG(dbgs() << "store's func is " << caller->getName() << "\n";);
    MemoryUseOrDef *mem = mssas[caller]->getMemoryAccess(&*storeInst);
    if (mem) {
        LLVM_DEBUG(dbgs() << *mem << "\n";);
        for (User *U : mem->users()) {
            if (MemoryUse *m = dyn_cast<MemoryUse>(U)) {
                LLVM_DEBUG(dbgs() << "user " << *m << "\n";);
                Instruction *memInst = m->getMemoryInst();
                LLVM_DEBUG(dbgs() << "user inst " << *memInst << "\n";);
                if (isa<LoadInst>(memInst)) {
                    out.push_back(memInst);
                }
            }
        }
    }

    return out;
}

/*
Unpick bitcasts etc. to find the root GEP instruction. (might not be
generalisable)
TODO: can one encounter loops in this search of the graph?
*/
void ScaleVariableTracing::findGEPs(Value *V, std::vector<Value *> &geps) {
    // LLVM_DEBUG(dbgs() << "ooo| finding GEPs for " << *V << "\n";);
    for (User *U : V->users()) {
        // LLVM_DEBUG(dbgs() << "ooo| U = " << *U << "\n";);
        // if (auto* Ugep = dyn_cast<GEPOperator>(U->stripPointerCasts())) {
        if (auto *Ugep = dyn_cast<GEPOperator>(U)) {
            // LLVM_DEBUG(dbgs() << "ooo| it's a gep" << "\n";);
            geps.push_back(Ugep); // found a gep, stop searching on this branch
        } else {
            // LLVM_DEBUG(dbgs() << "ooo| it's NOT a gep " << "\n";);
            findGEPs(U, geps); // otherwise, look another level down
        }
    }
}

/*
Test whether two GEP calls *look* the same,
i.e. the same target and offsets.
What the results of the GEP call would be in practice is not considered.
*/
bool ScaleVariableTracing::gepsAreEqual(GEPOperator *a, GEPOperator *b) {
    // LLVM_DEBUG(dbgs() << "     Comparing " << *a << " and " << *b;);

    bool areEqual = (a->getSourceElementType() == b->getSourceElementType()) &&
                    (a->getPointerOperandType() == b->getPointerOperandType());
    areEqual = areEqual && a->getPointerOperand()->stripPointerCasts() ==
                               b->getPointerOperand()->stripPointerCasts();
    areEqual = areEqual && a->getNumIndices() == b->getNumIndices();
    if (areEqual) {
        // go over both sets of indices simultaneously (they are same length)
        for (auto aIter = a->idx_begin(), bIter = b->idx_begin(),
                  aEnd = a->idx_end(), bEnd = b->idx_end();
             aIter != aEnd || bIter != bEnd; ++aIter, ++bIter) {
            areEqual = areEqual && *aIter == *bIter;
            // LLVM_DEBUG(dbgs() << "\n===| " << *aIter << " and " << *bIter <<
            // "\n";);
        }
    }

    /*if (areEqual) {
        LLVM_DEBUG(dbgs() << "; they are equal\n";);
    } else {
        LLVM_DEBUG(dbgs() << "; not equal\n";);
    }*/

    // things not checked:
    // a->getResultElementType()
    // a->getAddressSpace()
    // a->getPointerAddressSpace()

    return areEqual;
}

ScaleVariableTracing::Result
ScaleVariableTracing::perform(Module &M, ModuleAnalysisManager &MAM) {
    getAllMSSAResults(M, MAM, mssas);
    std::vector<Value *> scale_variables;

    // get scale variables from other analyses
    std::vector<Value *> library_scale_variables =
        MAM.getResult<LibraryScaleVariableDetectionPass>(M).scale_variables;
    llvm::SmallPtrSet<const Value *, 8> manual_annotation_scale_variables =
        MAM.getResult<ManualAnnotationSelectionPass>(M).values;

    // collate all found scale variables
    scale_variables.insert(scale_variables.end(),
                           library_scale_variables.begin(),
                           library_scale_variables.end());
    for (const Value *sv : manual_annotation_scale_variables)
        scale_variables.push_back(const_cast<Value *>(sv));

    // trace scale instructions originating from scale variables
    scale_graph *sg = createScaleGraph(scale_variables);

    errs() << "--------------------------------------------\n";

    errs() << "\nPrinting scale variable def-use chains\n";
    for (scale_node *v : sg->scale_vars) {
        std::unordered_set<scale_node *> visited;
        printTraces(v);
    }

    errs() << "--------------------------------------------\n";

    ScaleVariableTracing::Result res{*sg};
    return res;
}

} // namespace oft
