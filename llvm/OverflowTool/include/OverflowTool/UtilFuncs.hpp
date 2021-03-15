#pragma once

#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/Module.h"

#include <map>
#include <string>
#include <unordered_set>

namespace llvm {
class Value;
}

namespace oft {

struct scale_node;
class scale_graph;

std::string getFunctionName(llvm::Instruction *inst);
bool getAllMSSAResults(llvm::Module &M, llvm::ModuleAnalysisManager &MAM,
                       std::map<llvm::Function *, llvm::MemorySSA *> &mssas);
void printTraces(llvm::Value *start, scale_graph *sg, int depth = 0);
void printTraces(scale_node *node, int depth = 0);
void printTraces(scale_node *node, std::unordered_set<scale_node *> &visited,
                 int depth = 0);
void printValue(llvm::Value *V, int depth);

} // namespace oft
