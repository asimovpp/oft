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

#define DEBUG_TYPE "hello"

STATISTIC(HelloCounter, "Counts number of functions greeted");

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct Hello : public ModulePass {
    static char ID; // Pass identification, replacement for typeid
    Hello() : ModulePass(ID) {}

    bool runOnModule(Module &M) override {
      for (Module::iterator func = M.begin(), e = M.end(); func != e; ++func) {
          errs() << "Hello: ";
          errs().write_escaped(func->getName()) << '\n';
      }
      
      return false;
    }
  };
}

char Hello::ID = 0;
static RegisterPass<Hello> X("OverflowTest", "Hello World Pass test");

