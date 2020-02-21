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

        bool runOnModule(Module &M) override {
            for (Module::iterator func = M.begin(), e = M.end(); func != e; ++func) {
                //errs() << "Function: " << func->getName() << "\n"; 
                
                for (inst_iterator I = inst_begin(*func), e = inst_end(*func); I != e; ++I) {
                    //errs() << "I: " << *I << "\n"; 
                    //TODO: might have to use CallSite wrapper instead to also catch "function invokation"
                    if (auto *callInst = dyn_cast<CallInst>(&*I)) {
                        errs() << "I: " << *callInst  << " is a call Instruction\n"; 
                        //TODO: make this work with Fortran. 
                        //Currently the Fortran LLVM IR does a bitcast on every function before calling it, thus losing information about the original call.
                        std::string func_name = callInst->getCalledFunction()->getName().str();
                        if (mpi_scale_functions.find(func_name) != mpi_scale_functions.end()) {
                            errs() << "^^ is a scale function\n";
                            errs() << "and has scale variable: " << *(callInst->getOperand(1)) << "\n"; 
                            scale_variables.push_back(callInst->getOperand(1));
                            /*for (Use &U : callInst->operands()) {
                                Value *v = U.get();
                                errs() << v->getNumUses() << "\n";
                            }*/
                        }
                    }
                }
      
            }
            
            errs() << "Scale variables found:\n"; 
            for (Value* V : scale_variables) {
                errs() << *V << " used in " << V->getNumUses() << " places \n";
            }

            return false;
        }
    };
}

char AnalyseScale::ID = 0;
static RegisterPass<AnalyseScale> X("analyse_scale", "Analyse application scale variables");



/* this code used to be within the module loop


          errs().write_escaped(func->getName());
          if (mpi_scale_functions.find(func->getName().str()) != mpi_scale_functions.end()) {
            errs() << " This is a scale function!" << '\n';

            for (Use &U : 
           
            for (inst_iterator i = inst_begin(*func), e = inst_end(*func); i != e; ++i) {
                errs() << "Opcodes: " << *i << '\n';
            }

           
            
            //for (Function::iterator bb = func->begin(), e = func->end(); bb != e; ++bb ) {
            //    for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i) {
            //        errs() << "Opcode: " << i->getOpcodeName() << '\n';
            //for (User::op_iterator fop = i->op_begin(), e = i->op_end(); fop != e; ++fop) {
            //    errs() << "Arg: " << fop->getOperandNo() << '\n'; 
            //    //errs() << "Arg: " << *fop << '\n'; 
            //}
            //    }
            //}
           
            
            for (User::op_iterator fop = func->op_begin(), e = func->op_end(); fop != e; ++fop) {
                errs() << "Arg: " << fop->get()->getName() << '\n'; 
                errs() << "Arg: " << fop->getOperandNo() << '\n'; 
            }
            //for (Function::arg_iterator farg = func->arg_begin(), e = func->arg_end(); farg != e; ++farg) {
            //    errs() << "Arg: " << farg->getArgNo() << '\n'; 
            //}

            //for (Function::iterator bb = func->begin(), e = func->end(); bb != e; ++bb ) {
            //    for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i) {
            //        errs() << "Opcode: " << i->getOpcodeName() << '\n';
            //    }
            //}
          } else {
            errs() << '\n';
          }


*/
