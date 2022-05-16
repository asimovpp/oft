#include "OverflowTool/Analysis/ScaleVariableTracing.hpp"

#include "OverflowTool/Analysis/Passes/LibraryScaleVariableDetectionPass.hpp"
#include "OverflowTool/Analysis/Passes/ManualAnnotationSelectionPass.hpp"
#include "OverflowTool/Debug.hpp"
#include "OverflowTool/ScaleGraph.hpp"
#include "OverflowTool/UtilFuncs.hpp"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <iterator>
#include <unordered_set>

#define DEBUG_TYPE "oft-scale-var-tracing"

STATISTIC(NumLoops, "Number of loops");

using namespace llvm;

namespace oft {
/*
Create a scale graph based on the provided list of scale variables (starting
points to tracing).
*/
scale_graph *ScaleVariableTracing::createScaleGraph(
    const std::vector<Value *> &scale_variables) {
    scale_graph *sg = new scale_graph;

    for (Value *scale_var : scale_variables)
        sg->addvertex(scale_var, true);

    return sg;
}

void ScaleVariableTracing::trace(std::vector<Value *> scale_variables,
                                 scale_graph &sg) {
    // Iterate through scale variables and find all instructions which they
    // influence (scale instructions)
    for (Value *V : scale_variables) {
        OFT_DEBUG(dbgs() << "-----pointerTrack input: " << *V << "\n";); 
        ValueTrace vt(V); 
        auto results = followBwd(&vt);
        OFT_DEBUG(dbgs() << "----pointerTrack output: \n";); 

        std::unordered_set<Value *> visited;
        for (auto res : results) {
            auto refs = analyseTrace(res);
            for (auto r : refs) {
                sg.addvertex(r, false);
                sg.addedge(V, r);
                // TODO: could add in the intermediate GEP (result of bwd
                // trace), when present
                traceScaleInstructionsUpToCalls(r, visited, sg);
            }
        }
    }

    OFT_DEBUG(dbgs() << "tracing call sites\n";);
    // expand call instructions iteratively until no more changes occur
    unsigned int prevSize = 0;
    while (sg.get_size() - prevSize != 0) {
        std::vector<Value *> to_follow;
        for (auto &node : sg.graph) {
            if (isa<CallInst>(node.first)) {
                OFT_DEBUG(dbgs()
                              << "found call site " << *(node.first) << "\n";);
                std::vector<Value *> continuations =
                    traceCallInstruction(node.first, sg);
                to_follow.insert(to_follow.end(), continuations.begin(),
                                 continuations.end());
            }
        }

        for (Value *child : to_follow) {
            OFT_DEBUG(dbgs()
                          << "following call site via " << *(child) << "\n";);
            std::unordered_set<Value *> visited;
            traceScaleInstructionsUpToCalls(child, visited, sg);
        }

        prevSize = sg.get_size();
    }

    return;
}

/*
Connect a call site to the called function's body via argument positions in the
scale graph.
*/
std::vector<Value *>
ScaleVariableTracing::traceCallInstruction(Value *V, scale_graph &sg) {
    std::vector<Value *> children;

    if (CallInst *callInst = dyn_cast<CallInst>(V)) {
        for (scale_node *parent : sg.getvertex(V)->parents) {
            OFT_DEBUG(dbgs() << "checking call " << *callInst << " with arg "
                             << *parent->value << "\n";);
            for (auto arg_to_track : getCallArgs(callInst, parent->value)) {
                children.push_back(arg_to_track);
                sg.addvertex(arg_to_track, false);
                sg.addedge(V, arg_to_track);
            }
        }
    }

    return children;
}

/*
Get the values that match the argument position of arg in call instruction callInst.
One argument in the input may match multiple instructions in the called function's body,
e.g. if one calls some function fn like fn(x, x, y).
Functions without bodies or that are variadic are not followed.
*/
std::vector<Value *> ScaleVariableTracing::getCallArgs(CallInst *callInst,
                                                       Value *arg) {
    std::vector<Value *> out;
    Function *fp = callInst->getCalledFunction();
    if (fp == NULL) {
        fp = dyn_cast<Function>(
            callInst->getCalledValue()->stripPointerCasts());
    }
    if (fp->isDeclaration()) {
        OFT_DEBUG(dbgs() << "Function body not available. ( " << fp->getName()
                         << " )\n";);
    } else if (fp->isVarArg()) {
        OFT_DEBUG(dbgs() << "Function is variadic. Don't know how "
                            "to trace: "
                         << fp->getName() << "\n";);
    } else {
        for (unsigned int i = 0; i < callInst->getNumOperands(); ++i) {
            OFT_DEBUG(dbgs() << "checking " << i << "th operand "
                             << *(callInst->getOperand(i)) << "\n";);
            if (callInst->getOperand(i) == arg) {
                OFT_DEBUG(dbgs() << i << "th operand of " << fp->getName()
                                 << " matches " << *arg << "\n";);
                out.push_back( &*(fp->arg_begin() + i) );
            }
        }
    }
    return out;
}

/*
Trace scale variable (arg 1) and add visited nodes to scale graph.
Stop at call sites.
Already visited nodes are skipped the second time.
*/
void ScaleVariableTracing::traceScaleInstructionsUpToCalls(
    Value *V, std::unordered_set<Value *> &visited, scale_graph &sg) {
    OFT_DEBUG(dbgs() << "Visiting node " << visited.size() << "\t" << *V
                     << "\n";);
    if (visited.find(V) != visited.end()) {
        OFT_DEBUG(dbgs() << "Node " << *V
                         << " has already been visited. Skipping.\n";);
        return;
    }
    visited.insert(V);

    std::vector<Value *> children;
    for (User *U : V->users()) {
        children.push_back(U);
        sg.addvertex(U, false);
        sg.addedge(V, U);
    }

    // store instructions require MemSSA to connect them to their corresponding
    // load instructions in the chain
    if (StoreInst *storeInst = dyn_cast<StoreInst>(V)) {
        std::vector<Instruction *> memUses = getUsingInstr(storeInst);
        children.insert(children.end(), memUses.begin(), memUses.end());
        for (Instruction *I : memUses) {
            children.push_back(I);
            sg.addvertex(I, false);
            sg.addedge(V, I);
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
    OFT_DEBUG(dbgs() << "store's func is " << caller->getName() << "\n";);
    MemoryUseOrDef *mem = mssas[caller]->getMemoryAccess(&*storeInst);
    if (mem) {
        OFT_DEBUG(dbgs() << *mem << "\n";);
        for (User *U : mem->users()) {
            if (MemoryUse *m = dyn_cast<MemoryUse>(U)) {
                OFT_DEBUG(dbgs() << "user " << *m << "\n";);
                Instruction *memInst = m->getMemoryInst();
                OFT_DEBUG(dbgs() << "user inst " << *memInst << "\n";);
                if (isa<LoadInst>(memInst)) {
                    out.push_back(memInst);
                }
            }
        }
    }

    return out;
}

struct CallInstVisitor : public InstVisitor<CallInstVisitor> { 
    Function* targetFunc; 
    //SmallVector<const llvm::Value *, 8> callInsts;
    SmallVector<llvm::Value *, 8> callInsts;
    CallInstVisitor(Function* func) : targetFunc(func) {}

    void visitCallInst(llvm::CallInst &CInst) { 
        auto *func = CInst.getCalledFunction();

        // if it is a Fortran function call, this should return the Function
        if (!func) {
            func = llvm::cast<llvm::Function>(
                CInst.getCalledValue()->stripPointerCasts());
        }

        if (!func) {
            return;
        }

        if (func == targetFunc) {
            callInsts.push_back(&CInst);
        }
    }
};

SmallVector<ValueTrace *, 8> ScaleVariableTracing::followBwd(ValueTrace *vt) {
    // TODO: use ptr set instead?
    SmallVector<ValueTrace *, 8> results;
    ValueTrace *vt2 = followBwdUpToArg(vt);
    // TODO: add iteration to support multiple levels of function nesting
    if (auto *argV = dyn_cast<Argument>(vt2->getTail())) {
        auto calls = followArg(argV);
        for (auto call : calls) {
            // clone vt2, add argToFollow, pass to next function
            ValueTrace *vt3 = new ValueTrace(vt2);
            vt3->addValue(call);
            // TODO: can the casts be removed?
            Value *argToFollow = cast<Value>(*((cast<CallInst>(call))->arg_begin() + argV->getArgNo()));
            vt3->addValue(argToFollow);
            ValueTrace *followed = followBwdUpToArg(vt3);
            //errs() << "~~~call: " << *call
            //       << "\n~~~foll:" << *followed->getTail() << "\n";
            results.push_back(followed);
        }
    } else {
        results.push_back(vt2);
    }
    return results;
}

SmallVector<Value *, 8> ScaleVariableTracing::followArg(Value *V) {
    SmallVector<Value *, 8> results;
    if (auto *argV = dyn_cast<Argument>(V)) {
        Function* F = argV->getParent();
        CallInstVisitor callInstVisitor(F);
        callInstVisitor.visit(F->getParent());
        auto calls = callInstVisitor.callInsts;
        results.insert(results.end(), calls.begin(), calls.end());
    }
    return results;
}


ValueTrace *ScaleVariableTracing::followBwdUpToArg(ValueTrace *vt) {
    //TODO: are trace through cases covered in the original "createScaleGraph" function?
    SmallPtrSet<const Value *, 4> Visited;
    Value *V = vt->getTail();
    Visited.insert(V);
    while(1) {
        if (auto *bitcastV = dyn_cast<BitCastOperator>(V)) {
            // TODO: are there cases where I NEED to use stripPointerCast? (the main
            // issue with stripping is that it removes 0 0 geps, which I actually
            // want to keep)
            //V = bitcastV->stripPointerCasts();
            V = bitcastV->getOperand(0);
            OFT_DEBUG(dbgs() << "*** got bitcast operand 0. Now:" << *V << "\n";);
        } else if (auto *gepV = dyn_cast<GEPOperator>(V)) {
            V = gepV->getPointerOperand();
            OFT_DEBUG(dbgs() << "*** got GEP pointer operand. Now:" << *V << "\n";);
        } else if (auto *loadV = dyn_cast<LoadInst>(V)) {
            V = getStore(loadV);
            OFT_DEBUG(dbgs() << "*** followed Load. Now:" << *V << "\n";);
        } else if (auto *storeV = dyn_cast<StoreInst>(V)) {
            V = storeV->getValueOperand();
            OFT_DEBUG(dbgs() << "*** followed Store. Now:" << *V << "\n";);
        } else if (auto *argV = dyn_cast<Argument>(V)) {
            OFT_DEBUG(dbgs() << "*** Value is an argument: " << *V << "\n";);
            OFT_DEBUG(dbgs() << "*** Parent is: " << argV->getParent()->getName() << "\n";);
            OFT_DEBUG(dbgs() << "*** It is arg number: " << argV->getArgNo() << "\n";);
            // further tracing cannot be done within the Function
            V = argV;
        } else {
            OFT_DEBUG(dbgs() << "*** No rule to further follow: " << *V << "\n";);
        }
 
        //} else if (auto *constV = dyn_cast<Constant>(V)) {
        //    OFT_DEBUG(dbgs() << "*** Value is constant: " << *V << "\n";);
        //} else if (V->getType()->isPointerTy()) {
        //    OFT_DEBUG(dbgs() << "*** Value is pointer type: " << *V << "\n";);
        //} 
        
        //if (auto *instrV = dyn_cast<Instruction>(V)) {
        //    OFT_DEBUG(dbgs() << "*** Value is instruction: " << *V << "\n";);
        //}
        if (Visited.insert(V).second) {
            vt->addValue(V);
        } else {
            break;
        }
    } 

    return vt;
}


SmallVector<Value *, 8> ScaleVariableTracing::analyseTrace(ValueTrace *vt) {
    SmallVector<Value *, 8> results;

    std::vector<GEPOperator*> traceGeps;
    Value *root = nullptr;
    bool done = false;
    for (auto V : vt->trace) {
        OFT_DEBUG(dbgs() << "Reading: " << *V << "\n";); 
        if (done) {
            OFT_DEBUG(dbgs() << "Trying to continue when trace analysis should be complete.\n";);
            break;
        }

        if (isa<AllocaInst>(V)) {
            root = V;
            OFT_DEBUG(dbgs() << "Done 1. Alloca root."<< "\n";);
            done = true;
        } else if (isa<GlobalVariable>(V)) {
            root = V;
            OFT_DEBUG(dbgs() << "Done 2. Global root."<< "\n";);
            done = true;
        } else if (auto *gep = dyn_cast<GEPOperator>(V)) {
            OFT_DEBUG(dbgs() << "Found gep. " << "\n";);
            traceGeps.push_back(gep);
        } else if (auto *callInst = dyn_cast<CallInst>(V)) {
            Function *fp = callInst->getCalledFunction();
            if (fp == NULL) {
                fp = dyn_cast<Function>(
                    callInst->getCalledValue()->stripPointerCasts());
            }

            if (fp->getName() == "malloc") {
                OFT_DEBUG(dbgs() << "Done 4. malloc root.\n";);
                root = callInst;
                done = true;
            }
        } else {
            // TODO: put all "known skippables" here and have another rule for unknown scenarios
            OFT_DEBUG(dbgs() << "No rules for this value type.\n";);
        }
    }

    if (root == nullptr && traceGeps.empty()) {
        OFT_DEBUG(dbgs() << "No trackable found!\n";);
        return results;
    }

    OFT_DEBUG(dbgs() << "Root trackable is:\n";);
    if (traceGeps.empty()) {
        OFT_DEBUG(dbgs() << "Simple variable " << *root << "\n";);
        results.push_back(root);
    } else if (traceGeps.size() == 1) {
        GEPOperator *g = traceGeps.back();
        if (root == nullptr) {
            OFT_DEBUG(dbgs() << "Unexpected: root is nullptr!\n";);
        } else {
            OFT_DEBUG(dbgs() << "GEP " << *g << " with root " << *root << "\n";);
            std::vector<Value *> geps;
            std::unordered_set<Value *> visited;
            findGEPs(root, geps, visited);
            for (auto gg : geps) { 
                OFT_DEBUG(dbgs() << "Testing gep: " << *gg << "\n";);
                if (gepsAreEqual(cast<GEPOperator>(g), cast<GEPOperator>(gg), root, root)) {
                    OFT_DEBUG(dbgs() << "   is equal\n";);
                    results.push_back(gg);
                }
            }
        }
    } else {
        if (root == nullptr) {
            OFT_DEBUG(dbgs() << "Unexpected: root is nullptr!\n";);
        } else {
            OFT_DEBUG(dbgs() << "GEP nest with root " << *root << "\n";); 
            std::vector<Value *> geps;
            std::unordered_set<Value *> visited;

            // approach: find geps fwd and "unroll" all the sequential geps that were marked in the trace

            std::vector<Value*> traceRoots = {root};
            std::vector<Value*> nextTraceRoots;
            //traverse geps from first to last
            for (std::vector<GEPOperator*>::reverse_iterator g = traceGeps.rbegin(); g != traceGeps.rend(); ++g) {
                OFT_DEBUG(dbgs() << "finding geps going to " << **g << "\n";);
                for (Value * traceRoot : traceRoots) {
                    OFT_DEBUG(dbgs() << "finding geps starting from " << *traceRoot << "\n";);
                    std::vector<Value*> tmpTraceRoots;
                    findGEPs(traceRoot, tmpTraceRoots, visited);
                    for (auto gg : tmpTraceRoots) { 
                        OFT_DEBUG(dbgs() << "Testing gep: " << *gg << "\n";);
                        if (gepsAreEqual(cast<GEPOperator>(*g), cast<GEPOperator>(gg), traceRoot, traceRoot)) {
                            OFT_DEBUG(dbgs() << "   is equal\n";);
                            nextTraceRoots.push_back(gg);
                        }
                    }
                }
                traceRoots = nextTraceRoots;
                nextTraceRoots.clear();
            }
            results.insert(results.end(), traceRoots.begin(), traceRoots.end());
        }
    }

    
    return results;
}


/*
Use MemSSA to find the defining store instruction corresponding to a load instruction.
*/
Value *ScaleVariableTracing::getStore(LoadInst *loadInst) {
    Function *caller = loadInst->getParent()->getParent();
    OFT_DEBUG(dbgs() << "load inst function is " << caller->getName() << "\n";);
    MemoryUseOrDef *mem = mssas[caller]->getMemoryAccess(&*loadInst);
    if (mem) {
        OFT_DEBUG(dbgs() << *mem << "\n";);
        if (mem->getOptimizedAccessType() != MustAlias) {
            OFT_DEBUG(dbgs() << "Memory access is ambiguous (May). Returning load argument.\n";);
            return loadInst->getPointerOperand();
        }
        MemoryAccess *definition = mem->getDefiningAccess();
        OFT_DEBUG(dbgs() << "defining access: " << *definition << "\n";);
        return cast<MemoryUseOrDef>(definition)->getMemoryInst();
    }

    return nullptr;
}

/*
Unpick bitcasts etc. to find the root GEP instruction. (might not be
generalisable)
*/
void ScaleVariableTracing::findGEPs(Value *V, std::vector<Value *> &geps, std::unordered_set<Value *> &visited) {
    if (visited.find(V) != visited.end()) {
        return;
    }
    // call instructions should not be marked as visited, as the code is written now.
    // this allows entry via multiple args
    visited.insert(V);

    OFT_DEBUG(dbgs() << "findGEPs| finding GEPs for " << *V << "\n";);
    for (User *U : V->users()) {
        OFT_DEBUG(dbgs() << "findGEPs| U = " << *U << "\n";);
        if (auto *Ugep = dyn_cast<GEPOperator>(U)) {
            OFT_DEBUG(dbgs() << "findGEPs| it's a gep" << "\n";);
            geps.push_back(Ugep); // found a gep, stop searching on this branch
        } else if (StoreInst *storeInst = dyn_cast<StoreInst>(U)) {
            OFT_DEBUG(dbgs() << "findGEPs| it's a memory op" << "\n";);
            std::vector<Instruction *> memUses = getUsingInstr(storeInst);
            for (Instruction *I : memUses) {
                findGEPs(I, geps, visited);
            }
        } else if (CallInst *callInst = dyn_cast<CallInst>(U)) {
            OFT_DEBUG(dbgs() << "findGEPs| it's a call op" << "\n";);
            for (auto arg_to_track : getCallArgs(callInst, V)) {
				findGEPs(arg_to_track, geps, visited);
            }
        } else {
            OFT_DEBUG(dbgs() << "findGEPs| it's NOT a gep " << "\n";);
            findGEPs(U, geps, visited); // otherwise, look another level down
        }
    }
}

/*
Test whether two GEP calls *look* the same,
i.e. the same target and offsets.
What the results of the GEP call would be in practice is not considered.
*/
bool ScaleVariableTracing::gepsAreEqual(GEPOperator *a, GEPOperator *b) {
    // OFT_DEBUG(dbgs() << "     Comparing " << *a << " and " << *b;);

    bool areEqual = (a->getSourceElementType() == b->getSourceElementType()) &&
                    (a->getPointerOperandType() == b->getPointerOperandType());
    areEqual = areEqual && a->getPointerOperand()->stripPointerCasts() ==
                               b->getPointerOperand()->stripPointerCasts();
    areEqual = areEqual && a->getNumIndices() == b->getNumIndices();
    if (areEqual) {
        // go over both sets of indices simultaneously (they are same length)
        for (auto aIter = a->idx_begin(), bIter = b->idx_begin(),
                  aEnd = a->idx_end(), bEnd = b->idx_end();
             aIter != aEnd && bIter != bEnd; ++aIter, ++bIter) {
            areEqual = areEqual && *aIter == *bIter;
            // OFT_DEBUG(dbgs() << "\n===| " << *aIter << " and " << *bIter <<
            // "\n";);
        }
    }

    /*if (areEqual) {
        OFT_DEBUG(dbgs() << "; they are equal\n";);
    } else {
        OFT_DEBUG(dbgs() << "; not equal\n";);
    }*/

    // things not checked:
    // a->getResultElementType()
    // a->getAddressSpace()
    // a->getPointerAddressSpace()

    return areEqual;
}


void ScaleVariableTracing::followArithmeticBwd(ValueTrace *vt) {
    SmallPtrSet<const Value *, 4> Visited;
    Value *V = vt->getTail();
    Visited.insert(V);
    while(1) {
        if (auto *bitcastV = dyn_cast<BitCastOperator>(V)) {
            V = bitcastV->getOperand(0);
            OFT_DEBUG(dbgs() << "*** got bitcast operand 0. Now:" << *V << "\n";);
        } else if (auto *gepV = dyn_cast<GEPOperator>(V)) {
            V = gepV->getPointerOperand();
            OFT_DEBUG(dbgs() << "*** got GEP pointer operand. Now:" << *V << "\n";);
        } else if (auto *loadV = dyn_cast<LoadInst>(V)) {
            V = ScaleVariableTracing::getStore(loadV);
            OFT_DEBUG(dbgs() << "*** followed Load. Now:" << *V << "\n";);
        } else if (auto *storeV = dyn_cast<StoreInst>(V)) {
            V = storeV->getValueOperand();
            OFT_DEBUG(dbgs() << "*** followed Store. Now:" << *V << "\n";);
        } else if (auto *argV = dyn_cast<Argument>(V)) {
            OFT_DEBUG(dbgs() << "*** Value is an argument: " << *V << "\n";);
            OFT_DEBUG(dbgs() << "*** Parent is: " << argV->getParent()->getName() << "\n";);
            OFT_DEBUG(dbgs() << "*** It is arg number: " << argV->getArgNo() << "\n";);
            // further tracing cannot be done within the Function
            V = argV;
        } else if (auto *binV = dyn_cast<BinaryOperator>(V)) {
            OFT_DEBUG(dbgs() << "It's a binop\n";); 
            std::set<unsigned> binops = {llvm::Instruction::Add, llvm::Instruction::Sub,
                                         llvm::Instruction::Shl, llvm::Instruction::LShr,
                                         llvm::Instruction::AShr, llvm::Instruction::Mul};
            if (binops.count(binV->getOpcode())) {
                OFT_DEBUG(dbgs() << "  is a binop of interest\n";);
                auto op1 = binV->getOperand(0);
                auto op2 = binV->getOperand(1);
                if (isa<Constant>(op1) && isa<Constant>(op2)){
                    OFT_DEBUG(dbgs() << "Following both branches of a binary operator is not implemented.\n";);
                } else if (!isa<Constant>(op1)) {
                    V = op1;
                } else if (!isa<Constant>(op2)) {
                    V = op2;
                } else {
                    //can't trace further
                }
            }
        } else {
            OFT_DEBUG(dbgs() << "*** No rule to further follow: " << *V << "\n";);
        }
        if (Visited.insert(V).second) {
            vt->addValue(V);
        } else {
            break;
        }
    } 
}


bool ScaleVariableTracing::naiveCompare(Value *a, Value *b) {
    errs() << "a is : " << *a << "\n";
    errs() << "b is : " << *b << "\n";

    if (isa<LoadInst>(a) && isa<LoadInst>(b)) {
        auto *loadA = cast<LoadInst>(a);
        auto *loadB = cast<LoadInst>(b);
        return loadA->getPointerOperand() == loadB->getPointerOperand();
    } else if (isa<BinaryOperator>(a) && isa<BinaryOperator>(b)) {
        // this method is saying "if index 0 and index 1 is equal in 
        // the instrucion up to this point, then this instruction is the same as well."
        // A naive way would be to just compare the const operator, but that is insufficient. 
        // I think that this method will be able to handle nested binops too.
        // TODO: can I run into a loop somewhere due to recursive calls to indicesAreEqual?
        // TODO: a better method would just remember which equalities have been assessed so far. 
        // perhaps I can have that here by construction: let the order of the calls 
        // establish that everything previous is the same.
        auto *binA = cast<BinaryOperator>(a);
        auto *binB = cast<BinaryOperator>(b);
        return binA->getOpcode() == binB->getOpcode() &&
               indicesAreEqual(binA->getOperand(0), binB->getOperand(0)) &&
               indicesAreEqual(binA->getOperand(1), binB->getOperand(1)); 
    } else {
        return false;
    }
}


bool ScaleVariableTracing::indicesAreEqual(Value *a, Value *b) {
    OFT_DEBUG(dbgs() << "Checking " << *a << "\n         " << *b << "\n";);
    if (!isa<Constant>(a) && !isa<Constant>(b)) {
        ValueTrace vta(a); 
        ValueTrace vtb(b); 
        followArithmeticBwd(&vta);
        followArithmeticBwd(&vtb);

        //errs() << "Printing vta\n";
        //for (auto v : vta.trace)
        //    printValue(errs(), const_cast<Value *>(v), 0);
        //errs() << "Printing vtb\n";
        //for (auto v : vtb.trace)
        //    printValue(errs(), const_cast<Value *>(v), 0);
        
        bool areEqual = true;
        if (vta.getSize() == vtb.getSize()) {
            for (auto aIter = vta.rbegin(), bIter = vtb.rbegin(),
                      aEnd = vta.rend(), bEnd = vtb.rend();
                 aIter != aEnd && bIter != bEnd; ++aIter, ++bIter) {
                areEqual = areEqual && ((*aIter == *bIter) || naiveCompare(*aIter, *bIter));
            }
        }
        return areEqual;
    } else {
        return a == b;
    }
    //technically, there could be a third scenario where one is a Constant
    //but the other is not and then try to evaluate if it works out to be the same value.
    //-O2 would probably be able to do that calculation/simplifaction though 
    //so it will not reimplement it here at this time. 
}


bool ScaleVariableTracing::gepsAreEqual(GEPOperator *a, GEPOperator *b, Value *rA, Value *rB) {
    bool areEqual = true;
    Value* rootA = a->getPointerOperand()->stripPointerCasts();
    Value* rootB = b->getPointerOperand()->stripPointerCasts();
    if (rA != nullptr) {
        rootA = rA;
    }
    if (rB != nullptr) {
        rootB = rB;
    }

    areEqual = areEqual && rootA == rootB;
    areEqual = areEqual && a->getNumIndices() == b->getNumIndices();
    if (areEqual) {
        // go over both sets of indices simultaneously (they are same length)
        for (auto aIter = a->idx_begin(), bIter = b->idx_begin(),
                  aEnd = a->idx_end(), bEnd = b->idx_end();
             aIter != aEnd && bIter != bEnd; ++aIter, ++bIter) {
            //areEqual = areEqual && *aIter == *bIter;
            areEqual = areEqual && indicesAreEqual(*aIter, *bIter);
        }
    }

    return areEqual;
}


void ScaleVariableTracing::traceLoops(scale_graph &sg) {
    // traverse the scale instructions in the scale graph, starting from scale_vars.
    std::vector<scale_node *> nodes = sg.scale_vars;
    std::vector<scale_node *> visited;
    // store nodes-to-be-added in newNodes and add them to sg after traversal of sg is done
    std::vector<std::pair<Value *, Value *>> newNodes;
    while (nodes.size() > 0) {
        scale_node *n = nodes.back();
        if (std::find(visited.begin(), visited.end(), n) != visited.end()) {
            nodes.pop_back();
            continue;
        } else {
            visited.push_back(n);
            nodes.pop_back();
            for (scale_node *c : n->children)
                nodes.push_back(c);

            // all instructions within a loop L will be marked as scale dependent if
            // a scale instruction affects a branch instruction which conditionally branches
            // into a basic block belonging to L (according to LI) or the preheader of L
            if (Instruction *I = dyn_cast<Instruction>(n->value)) {
                if (auto *BI = dyn_cast<BranchInst>(I)) {
                    OFT_DEBUG(dbgs() << "Found a branch inst:" << *BI << "\n";);  
                    if (BI->isConditional() ) {
                        OFT_DEBUG(dbgs() << "Branch is conditional.\n";);  
                        LoopInfo *LI = lis[BI->getParent()->getParent()];
                        for (auto br : BI->successors()) {
                            Loop *L = LI->getLoopFor(br);
                            if (L) {
                                OFT_DEBUG(dbgs() << *br << " is a loop\n";); 
                                for (auto b = L->block_begin(), be = L->block_end();
                                     b != be; ++b) {
                                    for (Instruction &i : **b) {
                                        newNodes.push_back({BI, &i});
                                        printValue(dbgs(), &i, 1);
                                    }
                                }
                            } else {
                                // Loops have to be queried individually to find if
                                // br is a preheader of a loop
                                for (auto loop = LI->begin(), loopEnd = LI->end(); 
                                    loop != loopEnd; ++loop) {
                                    Loop *L = &**loop;
                                    auto preheader = L->getLoopPreheader();
                                    if (preheader && br == preheader) {
                                        OFT_DEBUG(dbgs() << *br <<  " is a loop preheader\n";);
                                        for (auto b = L->block_begin(), be = L->block_end();
                                             b != be; ++b) {
                                            for (Instruction &i : **b) {
                                                newNodes.push_back({BI, &i});
                                                printValue(dbgs(), &i, 1);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    for (auto newNode : newNodes) {
        sg.addvertex(newNode.second, false);
        sg.addedge(newNode.first, newNode.second);
    }
}

ScaleVariableTracing::Result
ScaleVariableTracing::perform(Module &M, ModuleAnalysisManager &MAM,
                              bool shouldTraceLoops) {
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
    auto *sg = createScaleGraph(scale_variables);
    trace(scale_variables, *sg);

    if (shouldTraceLoops) {
        getAllLIResults(M, MAM, lis);

        for (const auto &e : lis) {
            NumLoops += std::distance(e.second->begin(), e.second->end());
        }

        traceLoops(*sg);
    }

    return {*sg};
}

} // namespace oft
