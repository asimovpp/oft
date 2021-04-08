#include "OverflowTool/Analysis/Passes/ScaleOverflowIntegerDetectionPass.hpp"

#include "OverflowTool/Analysis/Passes/ScaleVariableTracingPass.hpp"
#include "OverflowTool/Analysis/ScaleOverflowIntegerDetection.hpp"
#include "OverflowTool/Config.hpp"
#include "OverflowTool/UtilFuncs.hpp"

#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#include <sstream>
#include <vector>

#define DEBUG_TYPE OFT_SCALEOVERFLOWINTDET_PASS_NAME
#define PASS_CMDLINE_OPTIONS_ENVVAR "SCALEOVERFLOWINTDET_CMDLINE_OPTIONS"

enum class Operations : int { all, mulOnly };

static llvm::cl::opt<Operations> OperationsMode(
    "oft-detect-operations",
    llvm::cl::desc("Operations to detect for potential overflow:"),
    llvm::cl::init(Operations::all),
    llvm::cl::values(
        clEnumValN(Operations::all, "all",
                   "All operations (add, sub, mul, shl, lshr, ashr)"),
        clEnumValN(Operations::mulOnly, "mul-only",
                   "Multiplication operation only (mul)")));

static llvm::cl::opt<std::string> PrintOverflowable(
    "oft-print-overflowable", llvm::cl::value_desc("filename"),
    llvm::cl::ValueOptional,
    llvm::cl::desc("print overflowable instructions (default: stdout"));

llvm::AnalysisKey oft::ScaleOverflowIntegerDetectionPass::Key;

namespace oft {

// new passmanager pass

ScaleOverflowIntegerDetectionPass::ScaleOverflowIntegerDetectionPass() {
    llvm::cl::ResetAllOptionOccurrences();
    llvm::cl::ParseEnvironmentOptions(DEBUG_TYPE, PASS_CMDLINE_OPTIONS_ENVVAR);
}

ScaleOverflowIntegerDetectionPass::Result
ScaleOverflowIntegerDetectionPass::run(llvm::Module &M,
                                       llvm::ModuleAnalysisManager &MAM) {
    auto graph = MAM.getResult<ScaleVariableTracingPass>(M).graph;

    std::vector<unsigned> overflowOps{llvm::Instruction::Mul};

    if (OperationsMode == Operations::all) {
        overflowOps.insert(std::end(overflowOps),
                           {llvm::Instruction::Add, llvm::Instruction::Sub,
                            llvm::Instruction::Shl, llvm::Instruction::LShr,
                            llvm::Instruction::AShr});
    }

    ScaleOverflowIntegerDetection detection(std::begin(overflowOps),
                                            std::end(overflowOps));
    auto res = detection.perform(M, graph);

    if (PrintOverflowable.getPosition() > 0) {
        auto osOrError = createTextFile(PrintOverflowable);

        if (!osOrError) {
            std::stringstream ss;
            ss << osOrError.getError();
            llvm::report_fatal_error(ss.str());
        }

        auto &stream = *osOrError.get();

        stream << "Overflowable " << detection.OverflowBitsThreshold
               << "-bit int instructions for module: " << M.getName() << "\n";
        for (const auto *v : res.overflowable) {
            stream << *v << "\n";
        }
    }

    return res;
}

} // namespace oft
