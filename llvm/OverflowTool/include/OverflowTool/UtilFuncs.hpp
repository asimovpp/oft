#pragma once

#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"

#include <map>
#include <memory>
#include <string>
#include <unordered_set>

namespace llvm {
class Value;
class StringRef;
class raw_ostream;
class raw_fd_ostream;
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
llvm::ErrorOr<std::string> makeAbsolutePath(llvm::StringRef Path);
llvm::ErrorOr<std::string> isPathToExistingRegularFile(llvm::StringRef Path);
llvm::ErrorOr<std::unique_ptr<llvm::raw_fd_ostream>>
createTextFile(llvm::StringRef Path,
               llvm::sys::fs::OpenFlags Flags = llvm::sys::fs::OF_Text);

} // namespace oft
