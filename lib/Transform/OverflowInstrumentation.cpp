//===- OverflowTrackig.cpp - Testing integer overflow analysis
//---------------===//
//
// No license at the moment.
// Justs Zarins
// j.zarins@epcc.ed.ac.uk
//
//===----------------------------------------------------------------------===//
//
// This class is for testing and learning on the way to integer overflow
// analysis.
//
//===----------------------------------------------------------------------===//

#include "OverflowTool/Transform/OverflowInstrumentation.hpp"

#include "OverflowTool/Analysis/Passes/ScaleOverflowIntegerDetectionPass.hpp"
#include "OverflowTool/Analysis/Passes/ScaleVariableTracingPass.hpp"
#include "OverflowTool/Debug.hpp"
#include "OverflowTool/UtilFuncs.hpp"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/User.h"
#include "llvm/Support/raw_ostream.h"

#include <set>
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "oft-instrumentation"

namespace oft {
/*
Insert instrumentation call for instruction I.
instr_id is passed to the instrumentation call to differentiate between
instrumented results in the output from the instrumented application.
*/
void OverflowInstrumentation::instrumentInstruction(Instruction *I,
                                                    unsigned int instr_id,
                                                    Function *instrumentFunc) {
    // see :
    // https://stackoverflow.com/questions/51082081/llvm-pass-to-insert-an-external-function-call-to-llvm-bitcode
    // ArrayRef< Value* >
    // arguments(ConstantInt::get(Type::getInt32Ty(I->getContext()), I, true));

    Value *counterVal =
        ConstantInt::get(I->getContext(), APInt(32, instr_id, true));
    std::vector<Value *> args = {counterVal, I};
    errs() << "ID " << instr_id << " given to ";
    printValue(dbgs(), I, 0);
    ArrayRef<Value *> argRef(args);
    // OFT_DEBUG(dbgs() << "Inserting func with type " <<
    // *(instrumentFunc->getFunctionType()) << "\n"; OFT_DEBUG(dbgs() << "Func
    // is " <<
    // *(instrumentFunc) << "\n";);
    Instruction *newInst = CallInst::Create(instrumentFunc, argRef, "");

    // auto* newInst = new CallInst(instrumentFunc, I,
    // "overflowInstrumentation", I); Instruction *newInst =
    // CallInst::Create(instrumentFunc, I, ""); Instruction *newInst = new
    // CallInst(instrumentFunc, I, "");
    // I->getParent()->getInstList().insertAfter(I, newInst);
    newInst->insertAfter(I);

    // Type *PtrTy = PointerType::getUnqual(Type::Int64Ty);
    // CastInst *CI = CastInst::Create(Instruction::BitCast, I, PtrTy, "");
    // newInst->insertAfter(CI);
}

/*
Insert instrumentation initialisation at the start of the main function.
*/
void OverflowInstrumentation::initInstrumentation(Module &M,
                                                  Function *initInstrumentFunc,
                                                  int table_len) {
    for (auto func = M.begin(), e = M.end(); func != e; ++func) {
        if (func->getName() == "main" || func->getName() == "MAIN_") {
            OFT_DEBUG(dbgs() << "Inserting instrumentation initialisation\n";);
            std::vector<Value *> args = {
                ConstantInt::get(M.getContext(), APInt(32, table_len, true))};
            ArrayRef<Value *> argRef(args);
            Instruction *newInst =
                CallInst::Create(initInstrumentFunc, argRef, "");
            BasicBlock &BB = func->getEntryBlock();
            Instruction *I = BB.getFirstNonPHIOrDbg();
            newInst->insertBefore(I);

            break;
        }
    }
}

/*
Insert instrumentation finalisation before a call to mpi_finalize.
It is assumed that this occurs near the exit of the application and that
mpi_finalize is called only once.
*/
void OverflowInstrumentation::finaliseInstrumentation(
    Module &M, Function *finaliseInstrumentFunc) {
    const std::set<std::string> mpi_finalize_functions = {
        "MPI_Finalize", "mpi_finalize_", "mpi_finalize_f08_"};

    llvm::SmallVector<llvm::Instruction *, 8> mpi_finalize_calls;

    for (auto func = M.begin(), e = M.end(); func != e; ++func) {
        for (auto I = inst_begin(*func), e = inst_end(*func); I != e; ++I) {
            if (isa<CallInst>(&*I) &&
                mpi_finalize_functions.find(getFunctionName(&*I)) !=
                    mpi_finalize_functions.end()) {
                mpi_finalize_calls.push_back(&*I);
            }
        }
    }

    OFT_DEBUG(dbgs() << "Found " << mpi_finalize_calls.size()
                     << " MPI_Finalize calls.\n";);

    for (auto *I : mpi_finalize_calls) {
        Instruction *newInst = CallInst::Create(finaliseInstrumentFunc,
                                                SmallVector<Value *, 0>{}, "");
        OFT_DEBUG(dbgs() << "Inserting instrumentation finalisation before "
                            "instruction "
                         << *I << " in function " << I->getFunction()->getName()
                         << "\n";);
        if (I->getDebugLoc()) {
            OFT_DEBUG(dbgs() << "    which is on line "
                             << I->getDebugLoc()->getLine() << " in file "
                             << I->getDebugLoc()->getFilename() << "\n";);
        }
        newInst->insertBefore(&*I);
    }
}

/*
Find the function pointer by name in the given module.
*/
Function *OverflowInstrumentation::findFunction(Module &M, std::string funcName,
                                                Type *retTy,
                                                ArrayRef<Type *> argTys) {
    FunctionType *fType = FunctionType::get(retTy, argTys, false);
    FunctionCallee callee = M.getOrInsertFunction(funcName, fType);
    Function *out = cast<Function>(callee.getCallee());
    // TODO: add check that the found function has the same signature as the one
    // looked for FunctionComparator::cmpType(fType, callee.getFunctionType());
    OFT_DEBUG(dbgs() << "Found " << funcName << " function\n";);
    return out;
}

void OverflowInstrumentation::instrument(
    llvm::ArrayRef<llvm::Instruction *> Overflowable) {
    // insert instrumentation after scale instructions, plus setup/teardown
    // calls for the instrumentation
    Function *instrumentFunc =
        findFunction(M, "oft_store_max_val", Type::getVoidTy(M.getContext()),
                     SmallVector<Type *, 2>{Type::getInt32Ty(M.getContext()),
                                            Type::getInt32Ty(M.getContext())});

    unsigned int instr_id = 0;
    for (auto *I : Overflowable) {
        instrumentInstruction(I, instr_id, instrumentFunc);
        instr_id++;
    }

    OFT_DEBUG(dbgs() << "--------------------------------------------\n";);

    Function *initFunc =
        findFunction(M, "oft_init_vals", Type::getVoidTy(M.getContext()),
                     SmallVector<Type *, 1>{Type::getInt32Ty(M.getContext())});
    initInstrumentation(M, initFunc, instr_id);

    Function *finaliseFunc =
        findFunction(M, "oft_print_max_vals", Type::getVoidTy(M.getContext()),
                     SmallVector<Type *, 1>{});
    finaliseInstrumentation(M, finaliseFunc);

    OFT_DEBUG(dbgs() << "--------------------------------------------\n";);

    return;
}

} // namespace oft
