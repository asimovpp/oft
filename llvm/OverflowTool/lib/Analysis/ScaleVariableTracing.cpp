#include "OverflowTracking/Analysis/ScaleVariableTracing.hpp"
#include "OverflowTracking/ScaleGraph.hpp"
#include "OverflowTracking/Analysis/Passes/LibraryScaleVariableDetectionPass.hpp"

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/raw_ostream.h"
#include "OverflowTracking/UtilFuncs.hpp"
#include "llvm/Support/Debug.h"


#include <unordered_set>

#define DEBUG_TYPE "oft-scale-var-tracing"

using namespace llvm;

namespace oft {
    /*
    Create a scale graph based on the provided list of scale variables (starting points to tracing).
    */
    scale_graph* ScaleVariableTracing::createScaleGraph(std::vector<Value*> scale_variables) {
        scale_graph* sg = new scale_graph;

        for (Value* scale_var : scale_variables) sg->addvertex(scale_var, true);
        
        // Iterate through scale variables and find all instructions which they influence (scale instructions)
        for (Value* V : scale_variables) {
            std::unordered_set<Value*> visited; 
            if (isa<AllocaInst>(V)) {
                errs() << "tracing scale variable (alloca): " << *V << "\n"; 
                traceScaleInstructionsUpToCalls(V, visited, sg);
            } else if (isa<GlobalVariable>(V)) { 
                errs() << "tracing scale variable (global): " << *V << "\n"; 
                traceScaleInstructionsUpToCalls(V, visited, sg);
            } else if (auto* gep = dyn_cast<GEPOperator>(V)) { 
                errs() << "tracing scale variable (GEP): " << *V << "\n"; 
                if (GlobalVariable* gv = dyn_cast<GlobalVariable>(gep->getPointerOperand()->stripPointerCasts())) {
                    //errs() << "xxx| gv = " << *gv << "\n";
                    std::vector<Value*> geps;
                    findGEPs(gv, geps);
                    for (auto* GVgep : geps) {
                        //errs() << "xxx| GVgep = " << *GVgep << "\n";
                        if (gepsAreEqual(cast<GEPOperator>(gep), cast<GEPOperator>(GVgep))) {
                            //need to connect the equivalent gep (UUgep) to the original scale variable (gep) in order to make the scale graph sensible.
                            //TODO: Is there a better way of doing this?
                            sg->addvertex(GVgep, false);
                            sg->addedge(gep, GVgep);
                            traceScaleInstructionsUpToCalls(GVgep, visited, sg);
                        }
                    }
                } else {
                    errs() << "No rule for tracing global variable that isn't a struct " << *gv << " coming from " << *V << "\n";
                }
            } else {
                errs() << "No rule for tracing value " << *V << "\n";
            }
        }
       
        errs() << "tracing call sites\n";
        //expand call instructions iteratively until no more changes occur 
        unsigned int prevSize = 0;
        while (sg->get_size() - prevSize != 0) {
            std::vector<Value*> to_follow; 
            for (auto& node : sg->graph) {
               //for (scale_node* c : it.second->parents) errs() << *(c->value) << "=+=";
                if (isa<CallInst>(node.first)) {
                    //errs() << "found call site " << *(node.first) << "\n";
                    std::vector<Value*> continuations = traceCallInstruction(node.first, sg);
                    to_follow.insert(to_follow.end(), continuations.begin(), continuations.end());
                }
            }
        
            for (Value* child : to_follow) {
                //errs() << "following call site via " << *(child) << "\n";
                std::unordered_set<Value*> visited; 
                traceScaleInstructionsUpToCalls(child, visited, sg);
            }
            
            prevSize = sg->get_size();
        }
   
        return sg;
    }
    

    /*
    Connect a call site to the called function's body via argument positions in the scale graph.
    */
    std::vector<Value*> ScaleVariableTracing::traceCallInstruction(Value* V, scale_graph* sg) {
        std::vector<Value*> children;
        
        //the tracing is continued across function calls through argument position
        if (CallInst* callInst = dyn_cast<CallInst>(V)) { //TODO: also check if the number of users is =0?
            for (scale_node* parent : sg->getvertex(V)->parents) {
                //errs() << "checking " << *callInst << " and user " << *V << "\n"; 
                for (unsigned int i = 0; i < callInst->getNumOperands(); ++i) {
                    //errs() << "checking " << i << "th operand which is " << *(callInst->getOperand(i)) << "\n"; 
                    if (callInst->getOperand(i) == parent->value) {
                        Function* fp = callInst->getCalledFunction();
                        if (fp == NULL)
                            fp = dyn_cast<Function>(callInst->getCalledValue()->stripPointerCasts());
                        //errs() << "V is " << i << "th operand of " << *callInst << "; Function is " << fp->getName() << "\n";
                        if (! fp->isDeclaration()) { //TODO: check number of arguments; some are variadic
                            //errs() << "     Tracing in function body of called function via " << i << "th argument. ( " << fp->getName()  << " )\n";
                            //followChain(fp->getArg(i), depth+1, visited);
                            Value* arg_to_track = &*(fp->arg_begin() + i);
                            children.push_back(arg_to_track);
                            sg->addvertex(arg_to_track, false);
                            sg->addedge(V, arg_to_track);
                        } else {
                            //errs() << "     Function body not available for further tracing. ( " << fp->getName()  << " )\n";
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
    void ScaleVariableTracing::traceScaleInstructionsUpToCalls(Value* V, std::unordered_set<Value*> & visited, scale_graph* sg) {
        errs() << "Visiting node " << visited.size() << "\t" << *V << "\n"; 
        if (visited.find(V) != visited.end()) {
            errs() << "Node " << *V << " has already been visited. Skipping.\n";
            return;
        }
        visited.insert(V);

        std::vector<Value*> children;
        for (User* U : V->users()) {
            children.push_back(U);
            sg->addvertex(U, false);
            sg->addedge(V, U);
        }

        //store instructions require MemSSA to connect them to their corresponding load instructions in the chain
        if (StoreInst* storeInst = dyn_cast<StoreInst>(V)) { //TODO: also check if the number of users is =0?
            std::vector<Instruction*> memUses = getUsingInstr(storeInst);
            children.insert(children.end(), memUses.begin(), memUses.end());
            for (Instruction* I : memUses) {
                children.push_back(I);
                sg->addvertex(I, false);
                sg->addedge(V, I);
            }
        } 

        for (Value* child : children) {
            traceScaleInstructionsUpToCalls(child, visited, sg);
        }
    }


    /*
    Use MemSSA to find load instructions corresponding to a store instruction.
    */
    std::vector<Instruction*> ScaleVariableTracing::getUsingInstr(StoreInst* storeInst) {
        std::vector<Instruction*> out;
        
        //parent of instruction is basic block, parent of basic block is function (?)
        Function* caller = storeInst->getParent()->getParent();
        errs() << "store inst functin is " << caller->getName() << "\n";
        MemoryUseOrDef *mem = mssas[caller]->getMemoryAccess(&*storeInst);
        if (mem) {
            errs() << *mem << "\n";
            for (User* U : mem->users()) {
                if (MemoryUse *m = dyn_cast<MemoryUse>(U)) {
                    errs() << "user " << *m << "\n";
                    Instruction *memInst = m->getMemoryInst();
                    errs() << "user inst " << *memInst << "\n";
                    if (isa<LoadInst>(memInst)) {
                        out.push_back(memInst);
                    }
                }
            }
        }

        return out;
    }


    /*
    Unpick bitcasts etc. to find the root GEP instruction. (might not be generalisable) 
    TODO: can one encounter loops in this search of the graph?
    */
     void ScaleVariableTracing::findGEPs(Value* V, std::vector<Value*>& geps) {
        //errs() << "ooo| finding GEPs for " << *V << "\n"; 
        for (User *U : V->users()) {
            //errs() << "ooo| U = " << *U << "\n"; 
            //if (auto* Ugep = dyn_cast<GEPOperator>(U->stripPointerCasts())) {
            if (auto* Ugep = dyn_cast<GEPOperator>(U)) {
                //errs() << "ooo| it's a gep" << "\n"; 
                geps.push_back(Ugep); //found a gep, stop searching on this branch
            } else {
                //errs() << "ooo| it's NOT a gep " << "\n"; 
                findGEPs(U, geps); //otherwise, look another level down
            }
        }
    } 
    

    /*
    Test whether two GEP calls *look* the same,
    i.e. the same target and offsets.
    What the results of the GEP call would be in practice is not considered.
    */
    bool ScaleVariableTracing::gepsAreEqual(GEPOperator* a, GEPOperator* b) {
        //errs() << "     Comparing " << *a << " and " << *b; 

        bool areEqual = true;
        areEqual = areEqual && a->getSourceElementType() == b->getSourceElementType();
        areEqual = areEqual && a->getPointerOperandType() == b->getPointerOperandType();
        areEqual = areEqual && a->getPointerOperand()->stripPointerCasts() == b->getPointerOperand()->stripPointerCasts();
        areEqual = areEqual && a->getNumIndices() == b->getNumIndices();
        if (areEqual) {
            // iterate over both sets if indices simultaneously (they are same length)
            for (auto aIter = a->idx_begin(), bIter = b->idx_begin(),
                    aEnd = a->idx_end(), bEnd = b->idx_end();
                    aIter != aEnd || bIter !=bEnd; 
                    ++aIter, ++bIter) {
                areEqual = areEqual && *aIter == *bIter;
                //errs() << "\n===| " << *aIter << " and " << *bIter << "\n"; 
            }
        }
        
        
        /*if (areEqual) {
            errs() << "; they are equal\n";
        } else {
        
            errs() << "; not equal\n";
        }*/


        //things not checked: 
        //a->getResultElementType() 
        //a->getAddressSpace() 
        //a->getPointerAddressSpace() 

        return areEqual; 
    }
        

    /*
    Pretty print scale graph starting from "start".
    */
    void ScaleVariableTracing::printTraces(Value* start, int depth, std::unordered_set<scale_node*> & visited, scale_graph* sg) {
        printTraces(sg->getvertex(start), depth, visited);
    }

    void ScaleVariableTracing::printTraces(scale_node* node, int depth, std::unordered_set<scale_node*> & visited) {
        if (visited.find(node) != visited.end()) {
            errs() << "Node " << *(node->value) << " already visited\n";
            return;
        }
        visited.insert(node);
        printValue(node->value, depth);
        for (scale_node* n : node->children) printTraces(n, depth+1, visited);
    }



    ScaleVariableTracing::Result
    ScaleVariableTracing::perform(Module &M, ModuleAnalysisManager &MAM) {
        getAllMSSAResults(M, MAM, mssas);
        std::vector<Value*> scale_variables = MAM.getResult<LibraryScaleVariableDetectionPass>(M).scale_variables;
        scale_graph* sg = createScaleGraph(scale_variables);

        errs() << "--------------------------------------------\n"; 
        
        errs() << "\nPrinting scale variable def-use chains\n"; 
        for (scale_node* v : sg->scale_vars) {
            std::unordered_set<scale_node*> visited;
            printTraces(v, 0, visited);
        }
    
        errs() << "--------------------------------------------\n"; 

        ScaleVariableTracing::Result res{*sg};
        return res;
    }

}
