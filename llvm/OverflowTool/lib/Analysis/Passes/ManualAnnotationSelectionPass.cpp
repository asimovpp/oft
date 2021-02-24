
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

llvm::Optional<annotation_entry_t>
parseAnnotationEntry(const std::string &EntryLine) {
    std::istringstream iss(EntryLine);
    std::string item;
    std::vector<std::string> splitEntry;

    while (std::getline(iss, item, ' ')) {
        splitEntry.emplace_back(item);
    }

    if (splitEntry[1] != "true" && splitEntry[1] != "false") {
        return llvm::None;
    }

    auto start = splitEntry.begin();
    std::advance(start, 2);

    std::vector<unsigned> args;

    std::transform(start, splitEntry.end(), std::back_inserter(args),
                   [](const auto &e) { return std::stoul(e); });

    annotation_entry_t entry{splitEntry[0],
                             splitEntry[1] == "true" ? true : false, args};

    return entry;
}

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
