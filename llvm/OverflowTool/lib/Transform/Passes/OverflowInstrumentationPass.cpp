#include "OverflowTool/Transform/Passes/OverflowInstrumentationPass.hpp"

#include "OverflowTool/Analysis/Passes/ScaleOverflowIntegerDetectionPass.hpp"
#include "OverflowTool/Config.hpp"
#include "OverflowTool/Transform/OverflowInstrumentation.hpp"
#include "OverflowTool/UtilFuncs.hpp"

#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/CommandLine.h"

#include <sstream>

#define DEBUG_TYPE OFT_OVERFLOWINSTRUMENTATION_PASS_NAME

static llvm::cl::opt<std::string> PrintBwdTraces(
    "oft-print-bwd-traces", llvm::cl::value_desc("filename"),
    llvm::cl::ValueOptional,
    llvm::cl::desc("print backward traces of the overflowable instructions (default: stdout"));

static llvm::cl::opt<std::string> PrintScaleGraph(
    "oft-print-scale-graph", llvm::cl::value_desc("filename"),
    llvm::cl::ValueOptional,
    llvm::cl::desc("print the raw scale graph (default: stdout"));

namespace oft {

// new passmanager pass

OverflowInstrumentationPass::OverflowInstrumentationPass() {}

llvm::PreservedAnalyses
OverflowInstrumentationPass::run(llvm::Module &M,
                                 llvm::ModuleAnalysisManager &MAM) {
    const auto res = MAM.getResult<ScaleOverflowIntegerDetectionPass>(M);

    OverflowInstrumentation oi(M);
    oi.instrument(res.overflowable);
    
    if (PrintBwdTraces.getPosition() > 0) {
        auto osOrError = createTextFile(PrintBwdTraces);

        if (!osOrError) {
            std::stringstream ss;
            ss << osOrError.getError();
            llvm::report_fatal_error(ss.str());
        }

        auto &stream = *osOrError.get();

        stream << "***Begin: marked overflowable backward traces\n";
        for (auto *o : res.overflowable) {
            stream << "Printing history for " << *o << "\n";
            printBwdTraces(stream, o, const_cast<scale_graph*>(&res.graph));
            stream << "\n";
        }
        stream << "***End: marked overflowable backward traces\n";
    }
    
    if (PrintScaleGraph.getPosition() > 0) {
        auto osOrError = createTextFile(PrintScaleGraph);

        if (!osOrError) {
            std::stringstream ss;
            ss << osOrError.getError();
            llvm::report_fatal_error(ss.str());
        }

        auto &stream = *osOrError.get();
        res.graph.print(stream);
    }
    
    return llvm::PreservedAnalyses::none();
}

} // namespace oft
