
//
//

#include "OverflowTool/Analysis/Passes/ManualAnnotationSelectionPass.hpp"

#include "OverflowTool/Analysis/ManualAnnotationSelection.hpp"
#include "OverflowTool/Config.hpp"
#include "OverflowTool/UtilFuncs.hpp"

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/Support/CommandLine.h"
// using llvm::cl::*

#include "llvm/Support/ErrorHandling.h"
// using llvm::report_fatal_error

#include "llvm/ADT/Optional.h"

#include <algorithm>
#include <sstream>

#define DEBUG_TYPE OFT_MANUALANNOTATIONSELECTION_PASS_NAME

llvm::AnalysisKey oft::ManualAnnotationSelectionPass::Key;

static llvm::cl::list<std::string> AnnotationFiles(
    "oft-annotation-files",
    llvm::cl::desc("list of files that provide annotation instructions"),
    llvm::cl::ZeroOrMore);

namespace oft {

// new passmanager pass

ManualAnnotationSelectionPass::ManualAnnotationSelectionPass() {
    std::vector<std::string> normAnnotationFiles;

    std::transform(std::begin(AnnotationFiles), std::end(AnnotationFiles),
                   std::back_inserter(normAnnotationFiles), [](const auto &e) {
                       auto pathOrErr = normalizePathToRegularFile(e);

                       if (auto ec = pathOrErr.getError()) {
                           llvm::report_fatal_error(
                               "\"" + e + "\"" +
                               " is not a regular file or does not exist");
                       }

                       return pathOrErr.get();
                   });

    for (auto &e : normAnnotationFiles) {
        llvm::dbgs() << e << "\n";
    }
}

ManualAnnotationSelectionPass::Result
ManualAnnotationSelectionPass::run(llvm::Module &CurModule,
                                   llvm::ModuleAnalysisManager &MAM) {
    ManualAnnotationSelection mas;
    mas.visit(CurModule);
    return mas.getAnnotated();
}

llvm::PreservedAnalyses
ManualAnnotationSelectionPrinterPass::run(llvm::Module &M,
                                          llvm::ModuleAnalysisManager &AM) {
    ManualAnnotationSelectionPass::Result &res =
        AM.getResult<ManualAnnotationSelectionPass>(M);

    OS << "Manual annotations for module: " << M.getName() << "\n";
    for (const auto &e : res.values) {
        OS << *e << "\n";
    }

    return llvm::PreservedAnalyses::all();
}

} // namespace oft
