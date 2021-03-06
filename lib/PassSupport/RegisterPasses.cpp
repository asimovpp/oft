//
//
//

#include "OverflowTool/Analysis/Passes/LibraryScaleVariableDetectionPass.hpp"
#include "OverflowTool/Analysis/Passes/ManualAnnotationSelectionPass.hpp"
#include "OverflowTool/Analysis/Passes/ScaleOverflowIntegerDetectionPass.hpp"
#include "OverflowTool/Analysis/Passes/ScaleVariableTracingPass.hpp"
#include "OverflowTool/Config.hpp"
#include "OverflowTool/Debug.hpp"
#include "OverflowTool/Transform/Passes/OverflowInstrumentationPass.hpp"
#include "OverflowTool/Util.hpp"

#include "llvm/IR/PassManager.h"
// using llvm::ModuleAnalysisManager
// using llvm::ModulePassManager

#include "llvm/Passes/PassBuilder.h"
// using llvm::PassBuilder

#include "llvm/Passes/PassPlugin.h"
// using llvmGetPassPluginInfo

#include "llvm/ADT/StringRef.h"
// using llvm::StringRef

#define DEBUG_TYPE "oft-plugin-registration"

// plugin registration for opt new passmanager

namespace {

void parseModuleAnalyses(llvm::ModuleAnalysisManager &MAM) {
#define MODULE_ANALYSIS(NAME, CREATE_PASS)                                     \
    OFT_DEBUG(llvm::dbgs() << "registering module analysis " << NAME           \
                           << "\n";);                                          \
    MAM.registerPass([]() { return CREATE_PASS; });

#include "Passes.def"

    return;
}

bool parseModulePipeline(llvm::StringRef Name, llvm::ModulePassManager &MPM,
                         llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
#define MODULE_ANALYSIS(NAME, CREATE_PASS)                                     \
    if (llvm::parseAnalysisUtilityPasses<                                      \
            std::remove_reference<decltype(CREATE_PASS)>::type>(NAME, Name,    \
                                                                MPM))          \
        return true;

#define MODULE_PASS(NAME, CREATE_PASS)                                         \
    if (Name == NAME) {                                                        \
        OFT_DEBUG(llvm::dbgs()                                                 \
                      << "registering module pass " << NAME << "\n";);         \
        MPM.addPass(CREATE_PASS);                                              \
        return true;                                                           \
    }

#include "Passes.def"

    return false;
}

void registerPasses(llvm::PassBuilder &PB) {
    PB.registerAnalysisRegistrationCallback(parseModuleAnalyses);
    PB.registerPipelineParsingCallback(parseModulePipeline);
}

} // namespace

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "OverflowToolPlugin",
            STRINGIFY(VERSION_STRING), registerPasses};
}
