//===- OverflowTrackig.cpp - Testing integer overflow analysis ---------------===//
//
// No license at the moment.
// Justs Zarins
// j.zarins@epcc.ed.ac.uk
//
//===----------------------------------------------------------------------===//
//
// This class is for testing and learning on the way to integer overflow analysis.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/User.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/DDG.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/ADT/ilist_iterator.h"

using namespace llvm;

#include <unordered_set>

namespace {
    struct AnalyseScale : public ModulePass {
        // List of scale functions in MPI. Names as found in C and Fortran.
        const std::unordered_set<std::string> mpi_scale_functions = {"MPI_Comm_size", "MPI_Comm_rank", "MPI_Group_size", "MPI_Group_rank",
                                                                 "mpi_comm_size_", "mpi_comm_rank_", "mpi_group_size_", "mpi_group_rank_"};  
        std::vector<Value*> scale_variables;
        static char ID; // Pass identification, replacement for typeid
        AnalyseScale() : ModulePass(ID) {}


        void printChain(Value* V, int depth) {
            if (StoreInst* storeInst = dyn_cast<StoreInst>(V)) { //TODO: also check if the number of users is =0?
                //errs() << *storeInst << " is a store instruction\n";
                Function* caller = storeInst->getParent()->getParent();
                MemorySSA &mssa = getAnalysis<MemorySSAWrapperPass>(*caller).getMSSA();
                MemoryUseOrDef *mem = mssa.getMemoryAccess(&*storeInst);
                if (mem) //<< if I don't have this check, another memory ref gets lost??
                    errs() << *mem << "\n";
                errs() << "getting mem users\n";
                for (User *UU : mem->users()) {
                //for (auto UU = mem->user_begin(), AE = mem->user_end(); UU != AE; ++UU) {
                    if (MemoryUse *m = dyn_cast<MemoryUse>(UU)) {
                        Instruction *memInstr = m->getMemoryInst();
                        errs() << *m << " ||| " << *memInstr << "\n";
                        if (LoadInst* loadInst = dyn_cast<LoadInst>(memInstr)) { //TODO: should I include other kinds?
                            
                            //These lines introduce an intermitten segfault but sometimes the code works?
                            // No, after further inspection it doesn't seem that way
                            //vvvvvvvvvvvvvvvvvvvvv
                            for (int i = 0; i < depth+1; ++i)
                                errs() << "    ";
                            int line_num = loadInst->getDebugLoc().getLine();
                            errs() << *loadInst << " on Line " << line_num << "\n";
                            //^^^^^^^^^^^^^^^^^^^^^
                            
                            printChain(loadInst, depth+1);
                        }
                    }
                }
                errs() << "got mem users\n";
            } else {
                errs() << "getting normal users\n";
                for (User *U : V->users()) {
                    if (depth == 0) {
                        errs() << "\n";
                        for (int i = 0; i < depth; ++i)
                            errs() << "    ";
                        errs() << *V << " is used in instructions:\n";
                    }
                    if (Instruction *Inst = dyn_cast<Instruction>(U)) {
                        for (int i = 0; i < depth+1; ++i)
                            errs() << "    ";
                        int line_num = Inst->getDebugLoc().getLine();
                        errs() << *Inst << " on Line " << line_num << "\n";
                    }
                    printChain(U, depth+1);
                }
                errs() << "got normal users\n";
            }

        }

        bool runOnModule(Module &M) override {
            
            // Iterate through all instructions and find the MPI scale calls
            // Then store pointers to the corresponding scale variables
            for (Module::iterator func = M.begin(), e = M.end(); func != e; ++func) {
                errs() << "Function: " << func->getName() << "\n"; 
                for (inst_iterator I = inst_begin(*func), e = inst_end(*func); I != e; ++I) {
                    //errs() << "I: " << *I << "\n"; 
                    //TODO: might have to use CallSite wrapper instead to also catch "function invokation"
                    if (auto *callInst = dyn_cast<CallInst>(&*I)) {
                        errs() << "I: " << *callInst  << " is a call Instruction\n"; 
                        //TODO: make this work with Fortran. 
                        //Currently the Fortran LLVM IR does a bitcast on every function before calling it, thus losing information about the original call.
                        //getCalledFunction() Returns the function called, or null if this is an indirect function invocation. 
                        
                        Function* fp =  callInst->getCalledFunction();
                        std::string func_name;
                        
                        if (fp == NULL) {
                            func_name = callInst->getCalledOperand()->stripPointerCasts()->getName().str();
                        } else {
                            func_name = callInst->getCalledFunction()->getName().str();
                        }
                        
                        if (mpi_scale_functions.find(func_name) != mpi_scale_functions.end()) {
                            errs() << "^^ is a scale function\n";
                            errs() << "and has scale variable: " << *(callInst->getOperand(1)->stripPointerCasts()) << "\n"; 
                            scale_variables.push_back(callInst->getOperand(1)->stripPointerCasts());
                            /*for (Use &U : callInst->operands()) {
                                Value *v = U.get();
                                errs() << v->getNumUses() << "\n";
                            }*/
                        }

                    }
                }
      
                
                // this can be either ...*AA = &get... or ...&AA = get...
                // will crash if called on some functions like MPI calls (probably because they didn't go through the compilation process
                //AliasAnalysis &AA = getAnalysisIfAvailable<AAResultsWrapperPass>()->getAAResults();
                
                
                // this check checks whether there is an actual function body attached, otherwise AA call will segfault
                // https://stackoverflow.com/questions/34260973/find-out-function-type-in-llvm
                if (0 && ! func->isDeclaration()) {
                    errs() << "Getting DDG for function  " << func->getName() << "\n"; 
                    
                    // the hard way...
                    //AliasAnalysis &AA = getAnalysis<AAResultsWrapperPass>(*func).getAAResults();
                    //ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>(*func).getSE();
                    //LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>(*func).getLoopInfo();
                    //DependenceInfo D(func, AA, SE, LI);
                    
                    DependenceInfo &DI = getAnalysis<DependenceAnalysisWrapperPass>(*func).getDI();
                    DataDependenceGraph DDG(*func, DI);
                    
                    errs() << "DDG name is " << DDG.getName() << "\n";
                    for (DataDependenceGraph::iterator node = DDG.begin(), e = DDG.end(); node != e; ++node) {
                        errs() << "DDG node:  " << *node << "\n";
                        errs() << "DDG node type:  " << (*node)->getKind() << "\n";
                        if (auto *simpleNode = dyn_cast<SimpleDDGNode>(*node)) {
                            errs() << "DDG node instr:  " << *(simpleNode->getFirstInstruction()) << "\n";
                            for (DDGNode::iterator edge = (*node)->begin(), e = (*node)->end(); edge != e; ++edge) {
                                errs() << "DDG edge:  " << (*edge)->getKind();
                                
                                if (auto *targNode = dyn_cast<SimpleDDGNode>( &((*edge)->getTargetNode()) )) {
                                    errs() << "; edge target: " << *(targNode->getFirstInstruction()) << "\n";
                                } else {
                                    errs() << "; (else) edge target: " << ((*edge)->getTargetNode().getKind()) <<"\n";
                                }

                            }
                        }
                    }
                    errs() << "DDG is built.\n"; 
                } else {
                    errs() << "Not getting AA for function " << func->getName() << "\n"; 
                }
            
            
                if (! func->isDeclaration()) {
                    MemorySSA &mssa = getAnalysis<MemorySSAWrapperPass>(*func).getMSSA();
                    for (inst_iterator I = inst_begin(*func), e = inst_end(*func); I != e; ++I) {
                        int line_num = -1;
                        if (I->getDebugLoc())
                            line_num = I->getDebugLoc().getLine();
                        
                        MemoryUseOrDef *mem = mssa.getMemoryAccess(&*I);
                        if (mem) {
                            errs() << *I << "\t||| " << *mem << "\t||| on source Line " << line_num << "\n";
                        } else {
                            errs() << *I << "\t||| " << "no MemSSA" << "\t||| on source Line " << line_num << "\n";
                        }
                    } 
                }
      
            }
            
            errs() << "Scale variables found:\n"; 
            for (Value* V : scale_variables) {
                errs() << *V << " used in " << V->getNumUses() << " places \n";
            }
            errs() << "--------------------------------------------\n"; 

            // Iterate through scale variables and find all instructions where they are used...
            for (Value* V : scale_variables) {
                printChain(V, 0);
            }
            


            // false indicates that this pass did NOT change the program
            return false;
        }

 
        //specify that we need Alias Analysis to be run before this pass can run
        void getAnalysisUsage(AnalysisUsage &AU) const override {
            // this pass needs alias analysis
            // (should this be addRequiredTransitive instead?)
            //AU.addRequired<AliasAnalysis>();
            AU.addRequired<AAResultsWrapperPass>();
            AU.addRequired<ScalarEvolutionWrapperPass>();
            AU.addRequired<LoopInfoWrapperPass>();
            AU.addRequired<DependenceAnalysisWrapperPass>();
            AU.addRequired<MemorySSAWrapperPass>();
            // this pass doesn't invalidate any subsequent passes
            AU.setPreservesAll();
        }
 
    
    
    
    };
}

char AnalyseScale::ID = 0;
static RegisterPass<AnalyseScale> X("analyse_scale", "Analyse application scale variables");



