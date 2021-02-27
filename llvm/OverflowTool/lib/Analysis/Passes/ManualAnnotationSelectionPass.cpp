//

#include "OverflowTool/Analysis/Passes/ManualAnnotationSelectionPass.hpp"

#include "OverflowTool/Analysis/ManualAnnotationSelection.hpp"
#include "OverflowTool/Config.hpp"
#include "OverflowTool/UtilFuncs.hpp"

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringRef.h"
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
    llvm::cl::OneOrMore);

namespace oft {

llvm::Optional<annotation_entry_t>
parseAnnotationEntry(const std::string &EntryLine) {
    std::istringstream iss(EntryLine);
    std::string item;
    std::vector<std::string> splitEntry;

    while (std::getline(iss, item, ' ')) {
        splitEntry.emplace_back(item);
    }

    if (splitEntry.size() < 2) {
        LLVM_DEBUG(llvm::dbgs() << "Entry is missing return value and argument "
                                   "specification\n";);
        return llvm::None;
    }

    llvm::StringRef retVal = splitEntry[1];
    retVal = retVal.trim();
    if (!retVal.equals_lower("true") && !retVal.equals_lower("false")) {
        LLVM_DEBUG(llvm::dbgs() << "Return value specification is missing\n";);
        return llvm::None;
    }

    auto start = splitEntry.begin();
    std::advance(start, 2);

    std::vector<unsigned> args;

    std::transform(start, splitEntry.end(), std::back_inserter(args),
                   [](const auto &e) { return std::stoul(e); });

    return annotation_entry_t{splitEntry[0],
                              retVal.equals_lower("true") ? true : false, args};
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
            llvm::dbgs() << "Could not parse line: \"" << line << "\"\n";
            fin.setstate(std::ios_base::failbit);
            break;
        }
    }

    return fin.eof() ? fin.eof() : !fin.fail();
}

// new passmanager pass

ManualAnnotationSelectionPass::ManualAnnotationSelectionPass() {

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

ManualAnnotationSelectionPass::Result
ManualAnnotationSelectionPass::run(llvm::Module &CurModule,
                                   llvm::ModuleAnalysisManager &MAM) {
    for (auto &e : normAnnotationFiles) {
        if (!processFile(e, [this](auto entryLine) {
                llvm::StringRef entryLineRef(entryLine);
                entryLineRef = entryLineRef.trim();

                if (entryLineRef == "") {
                    return true;
                }

                auto entry = parseAnnotationEntry(entryLineRef);

                if (entry.hasValue()) {
                    this->ParsedAnnotations.emplace_back(entry.getValue());
                    return true;
                }

                return false;
            })) {
            llvm::report_fatal_error("Error processing file: \"" + e + "\"");
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
