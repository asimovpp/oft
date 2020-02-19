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

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#include <unordered_set>

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct AnalyseScale : public ModulePass {
    // List of scale functions in MPI. Names as found in C and Fortran.
    std::unordered_set<std::string> mpi_scale_functions = {"MPI_Comm_size", "MPI_Comm_rank", "MPI_Group_size", "MPI_Group_rank",
                                                                 "mpi_comm_size_", "mpi_comm_rank_", "mpi_group_size_", "mpi_group_rank_"};  
    //const char mpi_scale_functions[8][100] = {"MPI_Comm_size", "MPI_Comm_rank", "MPI_Group_size", "MPI_Group_rank",
    //                                          "mpi_comm_size_", "mpi_comm_rank_", "mpi_group_size_", "mpi_group_rank_"};  
    static char ID; // Pass identification, replacement for typeid
    AnalyseScale() : ModulePass(ID) {}

    bool runOnModule(Module &M) override {
      for (Module::iterator func = M.begin(), e = M.end(); func != e; ++func) {
          errs().write_escaped(func->getName());
          if (mpi_scale_functions.find(func->getName().str()) != mpi_scale_functions.end()) {
            errs() << " This is a scale function!" << '\n';
          } else {
            errs() << '\n';
          }
      }
      
      return false;
    }
  };
}

char AnalyseScale::ID = 0;
static RegisterPass<AnalyseScale> X("analyse_scale", "Analyse application scale variables");

