//

#include "OverflowTool/Analysis/Passes/ManualAnnotationSelectionPass.hpp"

#include "OverflowTool/Analysis/ManualAnnotationSelection.hpp"
#include "OverflowTool/Config.hpp"
#include "OverflowTool/UtilFuncs.hpp"

#include "llvm/ADT/Optional.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"

#include <algorithm>
#include <fstream>
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

template <typename UnaryPredicateT>
bool processFile(const std::string &Filepath, UnaryPredicateT UPFunc) {
    std::ifstream fin;

    fin.open(Filepath);

    if (!fin) {
        LLVM_DEBUG(llvm::dbgs()
                   << "Could not open file: \"" << Filepath << "\"\n");
        return false;
    }

    std::string line;

    while (std::getline(fin, line)) {
        if (!UPFunc(line)) {
            LLVM_DEBUG(llvm::dbgs()
                           << "Could not parse line: \"" << line << "\"\n";);
            fin.setstate(std::ios_base::failbit);
        }
    }

    return !fin.fail();
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
