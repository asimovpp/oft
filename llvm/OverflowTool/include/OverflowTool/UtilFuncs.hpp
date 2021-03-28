#pragma once

#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorOr.h"

#include <map>
#include <string>
#include <unordered_set>

namespace llvm {
class Value;
class raw_ostream;
} // namespace llvm

namespace oft {

struct scale_node;
class scale_graph;

std::string getFunctionName(llvm::Instruction *inst);
bool getAllMSSAResults(llvm::Module &M, llvm::ModuleAnalysisManager &MAM,
                       std::map<llvm::Function *, llvm::MemorySSA *> &mssas);
void printTraces(llvm::raw_ostream &os, llvm::Value *start, scale_graph *sg,
                 int depth = 0);
void printTraces(llvm::raw_ostream &os, scale_node *node, int depth = 0);
void printTraces(llvm::raw_ostream &os, scale_node *node,
                 std::unordered_set<scale_node *> &visited, int depth = 0);
void printValue(llvm::raw_ostream &os, llvm::Value *V, int depth);
llvm::ErrorOr<std::string> normalizePathToRegularFile(const llvm::Twine &Path);

} // namespace oft
