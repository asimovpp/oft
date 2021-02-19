#pragma once
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/Module.h"

#include <map>
#include <string>

namespace oft {
std::string getFunctionName(llvm::Instruction *inst);
bool getAllMSSAResults(llvm::Module &M, llvm::ModuleAnalysisManager &MAM,
                       std::map<llvm::Function *, llvm::MemorySSA *> &mssas);
void printValue(llvm::Value *V, int depth);
} // namespace oft
