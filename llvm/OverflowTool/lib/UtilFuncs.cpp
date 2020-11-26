#include "OverflowTracking/UtilFuncs.hpp"

//TODO: without this there is a compile error relating to CallInst. Why?
#include "llvm/Analysis/MemorySSA.h"

using namespace llvm;

namespace oft {
        /*
        Find the function name coming from a call instruction.
        Has special cases C and Fortran LLVM IR.
        */
        std::string getFunctionName(Instruction* inst) {
            std::string func_name = "";
            if (auto *callInst = dyn_cast<CallInst>(inst)) {
                // getCalledFunction() Returns the function called, or null if this is an indirect function invocation. 
                Function* fp =  callInst->getCalledFunction();

                if (fp == NULL) {
                    // Fortran LLVM IR does some bitcast on every function before calling it, thus losing information about the original call.
                    //func_name = callInst->getCalledOperand()->stripPointerCasts()->getName().str();
                    func_name = callInst->getCalledValue()->stripPointerCasts()->getName().str();
                } else {
                    // in LLVM IR from C it is straightforward to get the function name
                    func_name = callInst->getCalledFunction()->getName().str();
                }
            }
            return func_name;
        }
    
}
