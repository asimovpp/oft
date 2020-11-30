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

#include "OverflowTracking/Analysis/Passes/ScaleVariableTracingPass.hpp"
#include "OverflowTracking/Analysis/Passes/ScaleOverflowIntegerDetectionPass.hpp"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/User.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#include <unordered_set>
#include <map>
#include <vector>
#include "OverflowTracking/Transform/OverflowTracking.hpp"
#include "OverflowTracking/ScaleGraph.hpp"
#include "OverflowTracking/UtilFuncs.hpp"

namespace oft {
    /*
    Insert instrumentation call for instruction I. 
    instr_id is passed to the instrumentation call to differentiate between instrumented results 
    in the output from the instrumented application.
    */
    void AnalyseScale::instrumentInstruction(Instruction* I, unsigned int instr_id, Function* instrumentFunc) {
        // see : https://stackoverflow.com/questions/51082081/llvm-pass-to-insert-an-external-function-call-to-llvm-bitcode
        //ArrayRef< Value* > arguments(ConstantInt::get(Type::getInt32Ty(I->getContext()), I, true));

        Value* counterVal = llvm::ConstantInt::get(I->getContext(), llvm::APInt(32, instr_id, true));
        std::vector<Value*> args = {counterVal, I};
        errs() << "ID " << instr_id << " given to "; 
        printValue(I, 0);
        ArrayRef< Value* > argRef(args);
        //errs() << "Inserting func with type " << *(instrumentFunc->getFunctionType()) << "\n";
        //errs() << "Func is " << *(instrumentFunc) << "\n";
        Instruction* newInst = CallInst::Create(instrumentFunc, argRef, "");
        
        //auto* newInst = new CallInst(instrumentFunc, I, "overflowInstrumentation", I);
        //Instruction *newInst = CallInst::Create(instrumentFunc, I, "");
        //Instruction *newInst = new CallInst(instrumentFunc, I, "");
        //I->getParent()->getInstList().insertAfter(I, newInst);
        newInst->insertAfter(I);


        //Type *PtrTy = PointerType::getUnqual(Type::Int64Ty);
        //CastInst *CI = CastInst::Create(Instruction::BitCast, I, PtrTy, "");
        //newInst->insertAfter(CI);
    }

    
    /*
    Insert instrumentation initialisation at the start of the main function.
    */
    void AnalyseScale::initInstrumentation(Module& M, Function* initInstrumentFunc) {
        for (Module::iterator func = M.begin(), e = M.end(); func != e; ++func) {
            if (func->getName() == "main" || func->getName() == "MAIN_") {
                errs() << "Inserting instrumentation initialisation\n";

                std::vector<Value*> args = {};
                ArrayRef< Value* > argRef(args);
                Instruction* newInst = CallInst::Create(initInstrumentFunc, argRef, "");
                BasicBlock& BB = func->getEntryBlock();
                Instruction* I = BB.getFirstNonPHIOrDbg(); 
                newInst->insertBefore(I);

                break; 
            }
        }
    }


    /*
    Insert instrumentation finalisation before a call to mpi_finalize.
    It is assumed that this occurs near the exit of the application and that mpi_finalize is called only once.
    */
    void AnalyseScale::finaliseInstrumentation(Module& M, Function* finaliseInstrumentFunc) {
        const std::unordered_set<std::string> mpi_finalize_functions = {"MPI_Finalize", "mpi_finalize_", "mpi_finalize_f08_"};

        for (Module::iterator func = M.begin(), e = M.end(); func != e; ++func) {
            //if (func->getName() == "main" || func->getName() == "MAIN_") 
            if (true) {
                for (inst_iterator I = inst_begin(*func), e = inst_end(*func); I != e; ++I) {
                    //if (isa<ReturnInst>(&*I)) 
                    if (isa<CallInst>(&*I) && mpi_finalize_functions.find(getFunctionName(&*I)) != mpi_finalize_functions.end()) {
                        std::vector<Value*> args = {};
                        ArrayRef< Value* > argRef(args);
                        Instruction* newInst = CallInst::Create(finaliseInstrumentFunc, argRef, "");
                        errs() << "Inserting instrumentation finalisation before line " << I->getDebugLoc()->getLine() << " in file " << I->getDebugLoc()->getFilename() <<  "\n";
                        newInst->insertBefore(&*I);
                    }
                }

                //break; 
            }
        }
    }

    
    /*
    Find the function pointer by name in the given module.
    */
    Function* AnalyseScale::findFunction(Module &M, std::string funcName) {
        Function* out = NULL;
        for (Module::iterator func = M.begin(), e = M.end(); func != e; ++func) {
            if (func->getName() == funcName) {
                errs() << "Found " << funcName << " function\n";
                out = &*func;
                break;
            }
        }
        return out;
    }

    
    PreservedAnalyses AnalyseScale::perform(Module &M, ModuleAnalysisManager &AM) {
        const auto overflowable_int_instructions = AM.getResult<ScaleOverflowIntegerDetectionPass>(M).overflowable_int_instructions;
    
        //insert instrumentation after scale instructions, plus setup/teardown calls for the instrumentation
        Function* instrumentFunc = findFunction(M, "store_max_val");
        unsigned int instr_id = 0;
        for (Instruction* I : overflowable_int_instructions) {
            instrumentInstruction(I, instr_id, instrumentFunc);
            instr_id++;
        }
        errs() << "--------------------------------------------\n"; 

        initInstrumentation(M, findFunction(M, "init_vals"));
        finaliseInstrumentation(M, findFunction(M, "print_max_vals"));
        errs() << "--------------------------------------------\n"; 

        scale_graph sg = AM.getResult<ScaleVariableTracingPass>(M).scale_graph;
        sg.text_print();

        return PreservedAnalyses::none();
    }
}
