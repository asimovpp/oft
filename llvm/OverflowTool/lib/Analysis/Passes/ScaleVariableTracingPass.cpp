#include "OverflowTool/Config.hpp"

//#include "OverflowTool/Analysis/Passes/LibraryScaleVariableDetectionPass.hpp"

//#include "OverflowTool/Analysis/LibraryScaleVariableDetection.hpp"

#include "OverflowTool/Analysis/Passes/ScaleVariableTracingPass.hpp"
#include "OverflowTool/Analysis/ScaleVariableTracing.hpp"
#include "OverflowTool/UtilFuncs.hpp"

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/PassManager.h"
#include "llvm/Support/CommandLine.h"
// using llvm::cl::ParseEnvironmentOptions
// using llvm::cl::ResetAllOptionOccurrences

#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#include <sstream>

#define DEBUG_TYPE OFT_SCALEVARTRACING_PASS_NAME
#define PASS_CMDLINE_OPTIONS_ENVVAR "SCALEVARTRACING_CMDLINE_OPTIONS"

static llvm::cl::opt<std::string> PrintScaleUsedef(
    "oft-print-scale-defuse", llvm::cl::value_desc("filename"),
    llvm::cl::ValueOptional,
    llvm::cl::desc("print scale var def-use chains (default: stdout"));

llvm::AnalysisKey oft::ScaleVariableTracingPass::Key;

namespace oft {

// new passmanager pass

ScaleVariableTracingPass::ScaleVariableTracingPass() {
    llvm::cl::ResetAllOptionOccurrences();
    llvm::cl::ParseEnvironmentOptions(DEBUG_TYPE, PASS_CMDLINE_OPTIONS_ENVVAR);
}

ScaleVariableTracingPass::Result
ScaleVariableTracingPass::run(llvm::Module &CurModule,
                              llvm::ModuleAnalysisManager &MAM) {
    ScaleVariableTracing pass;
    auto result = pass.perform(CurModule, MAM);

    if (PrintScaleUsedef.getPosition() > 0) {
        auto osOrError = createTextFile(PrintScaleUsedef);

        if (!osOrError) {
            std::stringstream ss;
            ss << osOrError.getError();
            llvm::report_fatal_error(ss.str());
        }

        for (auto *v : result.scale_graph.scale_vars) {
            printTraces(*osOrError.get(), v);
        }
    }

    return result;
}

// printer pass

llvm::PreservedAnalyses
ScaleVariableTracingPrinterPass::run(llvm::Module &M,
                                     llvm::ModuleAnalysisManager &AM) {
    ScaleVariableTracing::Result &res =
        AM.getResult<ScaleVariableTracingPass>(M);

    OS << "Scale variable tracing for module: " << M.getName() << "\n";
    res.scale_graph.print(OS);

    return llvm::PreservedAnalyses::all();
}

} // namespace oft
