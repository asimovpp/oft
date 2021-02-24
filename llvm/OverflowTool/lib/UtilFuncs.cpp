#include "OverflowTool/UtilFuncs.hpp"

#include "OverflowTool/ScaleGraph.hpp"

// TODO: without this there is a compile error relating to CallInst. Why?
#include "llvm/ADT/SmallString.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

#include <system_error>

using namespace llvm;

namespace oft {
/*
Find the function name coming from a call instruction.
Has special cases C and Fortran LLVM IR.
*/
std::string getFunctionName(Instruction *inst) {
    std::string func_name = "";
    if (auto *callInst = dyn_cast<CallInst>(inst)) {
        // getCalledFunction() Returns the function called, or null if this is
        // an indirect function invocation.
        Function *fp = callInst->getCalledFunction();

        if (fp == NULL) {
            // Fortran LLVM IR does some bitcast on every function before
            // calling it, thus losing information about the original call.
            // func_name =
            // callInst->getCalledOperand()->stripPointerCasts()->getName().str();
            func_name = callInst->getCalledValue()
                            ->stripPointerCasts()
                            ->getName()
                            .str();
        } else {
            // in LLVM IR from C it is straightforward to get the function name
            func_name = callInst->getCalledFunction()->getName().str();
        }
    }
    return func_name;
}

/*
Populate a map with MemorySSA results for all functions within a module.
*/
bool getAllMSSAResults(Module &M, ModuleAnalysisManager &MAM,
                       std::map<Function *, MemorySSA *> &mssas) {
    auto &FAM =
        MAM.getResult<llvm::FunctionAnalysisManagerModuleProxy>(M).getManager();

    for (auto &F : M) {
        if (F.isDeclaration()) {
            continue;
        }

        auto &MSSA = FAM.getResult<llvm::MemorySSAAnalysis>(F).getMSSA();
        mssas.insert(std::pair<Function *, MemorySSA *>(&F, &MSSA));
    }

    return true;
}

/*
Pretty print scale graph starting from "start".
*/
void printTraces(llvm::Value *start, scale_graph *sg, int depth) {
    std::unordered_set<scale_node *> visited;
    printTraces(sg->getvertex(start), visited, depth);
}

/*
Pretty print scale graph starting from "node".
*/
void printTraces(scale_node *node, int depth) {
    std::unordered_set<scale_node *> visited;
    printTraces(node, visited, depth);
}

/*
Pretty print scale graph starting from "node".
This overloads the above.
*/
void printTraces(scale_node *node, std::unordered_set<scale_node *> &visited,
                 int depth) {
    if (visited.find(node) != visited.end()) {
        errs() << "Node " << *(node->value) << " already visited\n";
        return;
    }
    visited.insert(node);
    printValue(node->value, depth);
    for (scale_node *n : node->children)
        printTraces(n, visited, depth + 1);
}

/*
Print an instruction along with some of its debug information.
Depth controls the indentation of the printed line.
*/
void printValue(Value *V, int depth) {
    // if (depth == 0) {
    //    errs() << *V <<"\n";
    //}
    int line_num = -1;
    StringRef fileName = "unknown";

    if (Instruction *Inst = dyn_cast<Instruction>(V)) {
        DILocation *loc = Inst->getDebugLoc();
        if (loc) {
            line_num = loc->getLine();
            fileName = loc->getFilename();
        }
    }

    errs() << "├";
    for (int i = 0; i < depth; ++i)
        errs() << "-";
    errs() << *V << " on Line " << line_num << " in file " << fileName << "\n";
}

/*
Converts a path to a regular file to an absolute path and checks that:
1) the file exists
2) the file is regular directory
*/
llvm::ErrorOr<std::string> normalizePathToRegularFile(const llvm::Twine &Path) {
    llvm::SmallString<128> AbsolutePath;
    Path.toVector(AbsolutePath);

    if (!llvm::sys::path::is_absolute(AbsolutePath)) {
        if (std::error_code ec = llvm::sys::fs::make_absolute(AbsolutePath)) {
            return ec;
        }
    }

    if (!llvm::sys::fs::exists(AbsolutePath) ||
        !llvm::sys::fs::is_regular_file(AbsolutePath)) {
        return std::make_error_code(std::errc::no_such_file_or_directory);
    }

    return AbsolutePath.str();
}

} // namespace oft
