#include "OverflowTool/Analysis/LibraryScaleVariableDetection.hpp"

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/raw_ostream.h"
#include "OverflowTool/UtilFuncs.hpp"
#include "llvm/Support/Debug.h"

#include <unordered_set>

#define DEBUG_TYPE "oft-library-scale-vars"

using namespace llvm;

namespace oft {
        
    /*
    Find scale variables that are initiated as a mpi_comm_rank or mpi_comm_size variable.
    */
    std::vector<Value*> LibraryScaleVariableDetection::findMPIScaleVariables(Function* func) { 
        // List of scale functions in MPI. Names as found in C and Fortran.
        const std::unordered_set<std::string> mpi_scale_functions = {"MPI_Comm_size", "MPI_Comm_rank", "MPI_Group_size", "MPI_Group_rank", "mpi_comm_size_", "mpi_comm_rank_", "mpi_group_size_", "mpi_group_rank_", "mpi_comm_size_f08_", "mpi_comm_rank_f08_", "mpi_group_size_f08_", "mpi_group_rank_f08_"};  
        std::vector<Value*> vars;

        // Iterate through all instructions and find the MPI rank/size calls
        // Then store pointers to the corresponding scale variables
        for (inst_iterator I = inst_begin(*func), e = inst_end(*func); I != e; ++I) {
            // TODO: might have to use CallSite wrapper instead to also catch "function invokation"
            if (auto *callInst = dyn_cast<CallInst>(&*I)) {
                if (mpi_scale_functions.find(getFunctionName(callInst)) != mpi_scale_functions.end()) {
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
                    } else if (auto* gep = dyn_cast<GEPOperator>(scale_var)) { 
                        errs() << *I << " sets scale variable (GEP): " << *scale_var << "\n"; 
                        vars.push_back(scale_var);
                    } else if (scale_var->getType()->isPointerTy()) { 
                        errs() << *I << " sets pointer type: " << *scale_var << "\n"; 
                        vars.push_back(scale_var);
                    } else {
                        errs() << *scale_var << " is not alloca or global" << "\n"; 
                        errs() << *callInst << " is the scale function that was called" << "\n"; 
                    }
                }
            }
        }
        return vars;
    }



    LibraryScaleVariableDetection::Result
    LibraryScaleVariableDetection::perform(Module &M, ModuleAnalysisManager &AM) {
        std::vector<Value*> scale_variables;
        for (Module::iterator func = M.begin(), e = M.end(); func != e; ++func) {
            errs() << "Looking for scale variables in Function: " << func->getName() << "\n"; 
        
            const std::unordered_set<std::string> functions_to_ignore = {"store_max_val", "init_vals", "print_max_vals"};
            if (functions_to_ignore.find(func->getName()) == functions_to_ignore.end()) {
                std::vector<Value*> func_scale_vars = findMPIScaleVariables(&*func);
                scale_variables.insert(scale_variables.end(), func_scale_vars.begin(), func_scale_vars.end());
            }
        }

        errs() << "\n--------------------------------------------\n"; 
        errs() << "Scale variables found:\n"; 
        for (Value* V : scale_variables) {
            printValue(V, 0);
        }
        errs() << "--------------------------------------------\n"; 

        LibraryScaleVariableDetection::Result res{scale_variables};
        return res;
    }

}
