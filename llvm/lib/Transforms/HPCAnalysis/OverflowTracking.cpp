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

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/User.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

//#include "llvm/Analysis/DDG.h"
#include "llvm/Analysis/MemorySSA.h"

using namespace llvm;

#include <unordered_set>

namespace {
    struct AnalyseScale : public ModulePass {
        // List of scale functions in MPI. Names as found in C and Fortran.
        const std::unordered_set<std::string> mpi_scale_functions = {"MPI_Comm_size", "MPI_Comm_rank", "MPI_Group_size", "MPI_Group_rank", "mpi_comm_size_", "mpi_comm_rank_", "mpi_group_size_", "mpi_group_rank_", "mpi_comm_size_f08_", "mpi_comm_rank_f08_", "mpi_group_size_f08_", "mpi_group_rank_f08_"};  
        static char ID; 
        const std::unordered_set<unsigned> overflow_ops = {Instruction::Add, Instruction::Sub, Instruction::Mul, Instruction::Shl, Instruction::LShr, Instruction::AShr};  
        Function* instrumentFunc;
        AnalyseScale() : ModulePass(ID) {}

        
        void printMemDefUseChain(Value* V, int i) {
            if (Instruction* Inst = dyn_cast<Instruction>(V)) {
                Function* caller = Inst->getParent()->getParent();
                MemorySSA &mssa = getAnalysis<MemorySSAWrapperPass>(*caller).getMSSA();
                MemoryUseOrDef *mem = mssa.getMemoryAccess(&*Inst);
                if (mem) {
                    errs() << i << " ||| " << *(mem->getMemoryInst()) << " ||| " << *mem << "\n";
                    for (User *U : mem->users()) {
                        printMemDefUseChain(U, i+1); 
                    }
                }
            } else if (MemoryUseOrDef* mem = dyn_cast<MemoryUseOrDef>(V)) {
                    errs() << i << " ||| " << *(mem->getMemoryInst()) << " ||| " << *mem << "\n";
                    for (User *U : mem->users()) {
                        printMemDefUseChain(U, i+1); 
                    }
            }
        }
        
        
        void printMemUseDefChain(Value* V, int i) {
            if (Instruction* Inst = dyn_cast<Instruction>(V)) {
                Function* caller = Inst->getParent()->getParent();
                MemorySSA &mssa = getAnalysis<MemorySSAWrapperPass>(*caller).getMSSA();
                MemoryUseOrDef *mem = mssa.getMemoryAccess(&*Inst);
                if (mem) {
                    errs() << i << " ||| " << *(mem->getMemoryInst()) << " ||| " << *mem << "\n";
                    for (Use &U : mem->operands()) {
                        printMemUseDefChain(U.get(), i-1); 
                    }
                } else {
                    errs() << i << " ||| " << "no Instruction" << " ||| " << *Inst << "\n";
                }
            } else if (MemoryUseOrDef* mem = dyn_cast<MemoryUseOrDef>(V)) {
                if (mem->getMemoryInst()) {
                    errs() << i << " ||| " << *(mem->getMemoryInst()) << " ||| " << *mem << "\n";
                    for (Use &U : mem->operands()) {
                        printMemUseDefChain(U.get(), i-1); 
                    }
                } else {
                    errs() << i << " ||| " << "no Instruction" << " ||| " << *mem << "\n";
                }
            }
        }


        bool canIntegerOverflow(Value* V) {
            //check that it is the right type of instruction and
            //that it is one of the instructions we care about
            //TODO: that it is operating with 32 bits (or fewer) 
            //TODO: what would happen if the operation was between 32 bit and 64 bit values? would the needed cast be in a separate instrucion somewhere?
            if (BinaryOperator* I = dyn_cast<BinaryOperator>(V)) {
                if (overflow_ops.find(I->getOpcode()) != overflow_ops.end() &&
                    I->getType()->isIntegerTy() &&
                    I->getType()->getScalarSizeInBits() <= 32) {
                    errs() << "Instruction " << *I << " could overflow. Has type " << *(I->getType()) << "\n"; 
                    return true;
                }
            }
            return false;
        }

        void instrumentInstruction(Instruction* I) {
            // see : https://stackoverflow.com/questions/51082081/llvm-pass-to-insert-an-external-function-call-to-llvm-bitcode
            //ArrayRef< Value* > arguments(ConstantInt::get(Type::getInt32Ty(I->getContext()), I, true));
            std::vector<Value*> args = {I};
            ArrayRef< Value* > argRef(args);
            errs() << "Inserting func with type " << *(instrumentFunc->getFunctionType()) << "\n";
            errs() << "Func is " << *(instrumentFunc) << "\n";
            Instruction* newInst = CallInst::Create(instrumentFunc, argRef, "");
            //TODO: this is just to appease the compiler. I should add the actual debug info for the function.
            // see: https://llvm.org/doxygen/classllvm_1_1DebugLoc.html#a4bccb0979d1d30e83fe142ac7fb4747b
            auto dl = I->getDebugLoc();
            newInst->setDebugLoc(dl);
            //auto* newInst = new CallInst(instrumentFunc, I, "overflowInstrumentation", I);
            //Instruction *newInst = CallInst::Create(instrumentFunc, I, "");
            //Instruction *newInst = new CallInst(instrumentFunc, I, "");
            //I->getParent()->getInstList().insertAfter(I, newInst);
            newInst->insertAfter(I);


            //Type *PtrTy = PointerType::getUnqual(Type::Int64Ty);
            //CastInst *CI = CastInst::Create(Instruction::BitCast, I, PtrTy, "");
            //newInst->insertAfter(CI);
        }


        std::vector<Instruction*> getUsingInstr(StoreInst* storeInst) {
            std::vector<Instruction*> out;

            //parent of instruction is basic block, parent of basic block is function (?)
            Function* caller = storeInst->getParent()->getParent();
            //errs() << "store inst functin is " << caller->getName() << "\n";
            MemorySSA &mssa = getAnalysis<MemorySSAWrapperPass>(*caller).getMSSA();
            MemoryUseOrDef *mem = mssa.getMemoryAccess(&*storeInst);
            if (mem) {
                //errs() << *mem << "\n";
                for (User* U : mem->users()) {
                    if (MemoryUse *m = dyn_cast<MemoryUse>(U)) {
                        Instruction *memInst = m->getMemoryInst();
                        if (isa<LoadInst>(memInst)) {
                            out.push_back(memInst);
                        }
                    }
                }
            }
            
            return out;
        }


        void printValue(Value* V, int depth) {
            if (depth == 0) {
                errs() << "\n";
             } 
             if (Instruction *Inst = dyn_cast<Instruction>(V)) {
                 for (int i = 0; i < depth; ++i)
                     errs() << "    ";
                 
                 int line_num = -1;
                 StringRef fileName = "unknown";
                 DILocation* loc = Inst->getDebugLoc();
                 if (loc) {
                     line_num = loc->getLine();
                     fileName = loc->getFilename();
                 }

                 errs() << *Inst << " on Line " << line_num << " in file " << fileName << "\n";
             } 
        }


       /* Argument* getFuncArg(Function* fp, unsigned int i) {
            unsigned int counter = 0;
            for (arg_iterator args = fp->arg_begin(), e = fp->arg_end(); args != e; ++args) {
                if (i == counter) 
                    return &*args;
                ++counter;
            }
            return NULL;
        }*/

        void followChain(Value* V, int depth) {
            printValue(V, depth);
            if (canIntegerOverflow(V)) {
                Instruction* VI = cast<Instruction>(V);
                instrumentInstruction(VI);
            }

            for (User *U : V->users()) {
                if (Instruction *Inst = dyn_cast<Instruction>(U)) {
                    followChain(U, depth+1);

                    if (StoreInst* storeInst = dyn_cast<StoreInst>(Inst)) { //TODO: also check if the number of users is =0?
                        std::vector<Instruction*> memUses = getUsingInstr(storeInst);
                        for (std::vector<Instruction*>::iterator it = memUses.begin(); it != memUses.end(); ++it) {
                            followChain(*it, depth+2);
                        }
                    } 

                    if (CallInst* callInst = dyn_cast<CallInst>(Inst)) { //TODO: also check if the number of users is =0?
                        //errs() << "checking " << *callInst << " and user " << *V << "\n"; 
                        for (unsigned int i = 0; i < callInst->getNumOperands(); ++i) {
                            //errs() << "checking " << i << "th operand which is " << *(callInst->getOperand(i)) << "\n"; 
                            if (callInst->getOperand(i) == V) {
                                errs() << "V is " << i << "th operand of " << *callInst << "\n";
                                Function* fp =  callInst->getCalledFunction();
                                if (fp == NULL)
                                    fp = dyn_cast<Function>(callInst->getCalledValue()->stripPointerCasts());
                                errs() << "Function is " << fp->getName() << "\n";
                                if (! fp->isDeclaration()) { //TODO: check number of arguments; some are variadic
                                    //followChain(fp->getArg(i), depth+1);
                                    followChain(&*(fp->arg_begin() + i), depth+1);
                                }
                            }
                        }
                    }
                }
            }
        }

       
        /// I think a recursive depth-first(?) application will end up giving the single deepest definiton.
        Value* findFirstDef(Value* v) {
            errs() << "checking " << *v << "\n"; 
            Value* out = v;
            if (Instruction *scI = dyn_cast<Instruction>(v)) {
                if (scI->getNumOperands() != 0) {
                    for (Use &U : scI->operands()) {
                        Value *next_v = U.get();
                        out = findFirstDef(next_v);
                    }
                }
            } else {
                errs() << "ignoring " << *v << " becuse not an Instruction\n"; 
            }

            return out;
        }

        std::vector<Value*> findMPIScaleVariables(Function* func) { 
             std::vector<Value*> vars;
             
            // Iterate through all instructions and find the MPI rank/size calls
            // Then store pointers to the corresponding scale variables
            for (inst_iterator I = inst_begin(*func), e = inst_end(*func); I != e; ++I) {
                // TODO: might have to use CallSite wrapper instead to also catch "function invokation"
                if (auto *callInst = dyn_cast<CallInst>(&*I)) {
                    // getCalledFunction() Returns the function called, or null if this is an indirect function invocation. 
                    Function* fp =  callInst->getCalledFunction();
                    std::string func_name;
                    
                    if (fp == NULL) {
                        // Fortran LLVM IR does some bitcast on every function before calling it, thus losing information about the original call.
                        //func_name = callInst->getCalledOperand()->stripPointerCasts()->getName().str();
                        func_name = callInst->getCalledValue()->stripPointerCasts()->getName().str();
                    } else {
                        // in LLVM IR from C it is straightforward to get the function name
                        func_name = callInst->getCalledFunction()->getName().str();
                    }
                    
                    if (mpi_scale_functions.find(func_name) != mpi_scale_functions.end()) {
                        // the scale variable is always the 2nd operand in the MPI functions of interest
                        Value* scale_var = callInst->getOperand(1)->stripPointerCasts();
                        //Value* firstDef = findFirstDef(scale_var);
                        //errs() << *scale_var << " oldest ref is: " << *firstDef << "\n"; 

                        if (isa<AllocaInst>(scale_var)) {
                            errs() << *I << " sets scale variable (alloca): " << *scale_var << "\n"; 
                            vars.push_back(scale_var);
                        } else if (isa<GlobalVariable>(scale_var)) { 
                            errs() << *I << " sets scale variable (global): " << *scale_var << "\n"; 
                            vars.push_back(scale_var);
                        } else if (auto* gep = dyn_cast<GetElementPtrInst>(scale_var)) { 
                            errs() << "test: " << *(gep) << "\n"; 
                            errs() << "test2: " << *(gep->getPointerOperand()) << "\n"; 
                            errs() << "test3: " << *(gep->getPointerOperand()->stripPointerCasts()) << "\n"; 
                            
                            errs() << "test4: "<< "\n"; 
                            printMemDefUseChain(gep, 0);
                            printMemUseDefChain(gep, 0);
                            errs() << "test5: "<< "\n"; 
                            printMemDefUseChain(&*I, 0);
                            printMemUseDefChain(&*I, 0);
                            vars.push_back(gep);
                        } else {
                            errs() << *scale_var << " is not alloca or global" << "\n"; 
                        }
                    }
                }
            }

             return vars;
         }


/*        void printDDG(Function* func) {
                // this check checks whether there is an actual function body attached, otherwise the DDG call will segfault
                // https://stackoverflow.com/questions/34260973/find-out-function-type-in-llvm
                if (! func->isDeclaration()) {
                    errs() << "Getting DDG for function  " << func->getName() << "\n"; 
                    
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
                    errs() << "Not getting DDG for function " << func->getName() << "\n"; 
                }
        }*/


        void dumpInstrAndMemorySSA(Function* func) {
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
                        printMemDefUseChain(&*I, 0);
                    } 
                }
        }


        bool gepsAreEqual(GetElementPtrInst* a, GetElementPtrInst* b) {
            errs() << "Comparing " << *a << " and " << *b << "\n"; 
            
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
                }
            }
            
            
            //things not checked: 
            //a->getResultElementType() 
            //a->getAddressSpace() 
            //a->getPointerAddressSpace() 
        
            return areEqual; 
        }

        
        // some code duplication with followChain here. TODO: merge
        // WIP
        void findGEPs(Value* V, int depth) {
            for (User *U : V->users()) {
                if (Instruction *Inst = dyn_cast<Instruction>(U)) {
                    followChain(U, depth+1);

                    if (StoreInst* storeInst = dyn_cast<StoreInst>(Inst)) { //TODO: also check if the number of users is =0?
                        std::vector<Instruction*> memUses = getUsingInstr(storeInst);
                        for (std::vector<Instruction*>::iterator it = memUses.begin(); it != memUses.end(); ++it) {
                            followChain(*it, depth+2);
                        }
                    } 
                }
            }
        }



//======================================================

        bool runOnModule(Module &M) override {
            std::vector<Value*> scale_variables;
            for (Module::iterator func = M.begin(), e = M.end(); func != e; ++func) {
                errs() << "Function: " << func->getName() << "\n"; 
                
                std::vector<Value*> func_scale_vars = findMPIScaleVariables(&*func);
                scale_variables.insert(scale_variables.end(), func_scale_vars.begin(), func_scale_vars.end());

                if (func->getName() == "llvm_analysis_funcs_print_val_" || func->getName() == "print_val") {
                    errs() << "Found print_val function\n";
                    instrumentFunc = &*func;
                }
                
                //printDDG(&*func);
                //dumpInstrAndMemorySSA(&*func);
      
            }
            
            errs() << "\n--------------------------------------------\n"; 
            errs() << "Scale variables found:\n"; 
            for (Value* V : scale_variables) {
                errs() << *V << " used in " << V->getNumUses() << " places \n";
            }
            errs() << "--------------------------------------------\n"; 

            // Iterate through scale variables and find all instructions where they are used
            errs() << "\nPrinting scale variable def-use chains\n"; 
            for (Value* V : scale_variables) {
                if (isa<AllocaInst>(V)) {
                    errs() << "tracing scale variable (alloca): " << *V << "\n"; 
                    followChain(V, 0);
                } else if (isa<GlobalVariable>(V)) { 
                    errs() << "tracing scale variable (global): " << *V << "\n"; 
                    followChain(V, 0);
                } else if (auto* gep = dyn_cast<GetElementPtrInst>(V)) { 
                    // need a function here to prune the global variable accesses to only those we care about
                    // recurcively climb the def-use chain starting at gv
                    // when you get to a GEP instruction (normally only a couple of levels down), check that uses the same parameters as the scale instr
                    // need to examine the GEP in a bit more detail to (a) see what shapes it can take and (b) how to access the relevant fields
                    
                    errs() << "tracing scale variable (GEP): " << *V << "\n"; 
                    /*errs() << "t1: " << *(gep->getSourceElementType()) << "\n"; 
                    errs() << "t2: " << *(gep->getResultElementType()) << "\n"; 
                    errs() << "t3: " << gep->getAddressSpace() << "\n"; 
                    errs() << "t4: " << *(gep->getPointerOperand()) << "\n"; 
                    errs() << "t5: " << *(gep->getPointerOperandType()) << "\n"; 
                    errs() << "t6: " << gep->getPointerAddressSpace() << "\n"; 
                    for (auto ii = gep->idx_begin(), e = gep->idx_end(); ii != e; ++ii) {
                        errs() << "iterations: " << **ii << "\n"; 
                    }*/
                    
                    if (GlobalVariable* gv = dyn_cast<GlobalVariable>(gep->getPointerOperand()->stripPointerCasts())) {
                        // quick but maybe unreliable way to do it. TODO: implement "findGEPs" function
                        for (User* U : gv->users()) { //bitcast instr
                            for (User* UU : U->users()) { //usually GEP
                                if (auto* UUgep = dyn_cast<GetElementPtrInst>(UU)) { 
                                    if (gepsAreEqual(gep, UUgep)) {
                                        followChain(UUgep, 0);
                                    }
                                }
                            }
                        }
                        
                        
                    } else {
                        errs() << "No rule for tracing global variable that isn't a struct " << *gv << " coming from " << *V << "\n";
                    }
                } else {
                    errs() << "No rule for tracing value " << *V << "\n";
                }
            }

            // false indicates that this pass did NOT change the program
            return true;
        }

 
        //specify other passes that this pass depends on
        void getAnalysisUsage(AnalysisUsage &AU) const override {
            // (should this be addRequiredTransitive instead?)
            //AU.addRequired<DependenceAnalysisWrapperPass>();
            AU.addRequired<MemorySSAWrapperPass>();
            // this pass doesn't invalidate any subsequent passes
            // AU.setPreservesAll();
        }
 
    
    
    
    };
}

char AnalyseScale::ID = 0;
static RegisterPass<AnalyseScale> X("analyse_scale", "Analyse application scale variables");
