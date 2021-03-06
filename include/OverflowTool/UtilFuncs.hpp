#pragma once

#include "llvm/IR/PassManager.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"

#include <map>
#include <memory>
#include <string>
#include <unordered_set>

namespace llvm {
class Module;
class Function;
class Instruction;
class Value;
class StringRef;
class MemorySSA;
class raw_ostream;
class raw_fd_ostream;
class LoopInfo;
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

void printBwdTraces(llvm::raw_ostream &os, llvm::Value *start, scale_graph *sg,
                 int depth = 0);
void printBwdTraces(llvm::raw_ostream &os, scale_node *node, int depth = 0);
void printBwdTraces(llvm::raw_ostream &os, scale_node *node,
                 std::unordered_set<scale_node *> &visited, int depth = 0);

void printValue(llvm::raw_ostream &os, llvm::Value *V, int depth = 0);
llvm::ErrorOr<std::string> makeAbsolutePath(llvm::StringRef Path);
llvm::ErrorOr<std::string> isPathToExistingRegularFile(llvm::StringRef Path);
llvm::ErrorOr<std::unique_ptr<llvm::raw_fd_ostream>>
createTextFile(llvm::StringRef Path,
               llvm::sys::fs::OpenFlags Flags = llvm::sys::fs::OF_Text);
bool getAllLIResults(llvm::Module &M, llvm::ModuleAnalysisManager &MAM,
                     std::map<llvm::Function *, llvm::LoopInfo *> &lis);

} // namespace oft
