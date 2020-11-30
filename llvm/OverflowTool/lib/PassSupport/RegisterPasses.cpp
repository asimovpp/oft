//
//
//

#include "OverflowTool/Config.hpp"

#include "OverflowTool/Util.hpp"

#include "OverflowTool/Analysis/Passes/ManualAnnotationSelectionPass.hpp"

#include "OverflowTracking/Transform/Passes/OverflowTrackingPass.hpp"

#include "OverflowTracking/Analysis/Passes/LibraryScaleVariableDetectionPass.hpp"

#include "OverflowTracking/Analysis/Passes/ScaleVariableTracingPass.hpp"

#include "OverflowTracking/Analysis/Passes/ScaleOverflowIntegerDetectionPass.hpp"

#include "llvm/IR/PassManager.h"
// using llvm::ModuleAnalysisManager
// using llvm::ModulePassManager

#include "llvm/Passes/PassBuilder.h"
// using llvm::PassBuilder

#include "llvm/Passes/PassPlugin.h"
// using llvmGetPassPluginInfo

#include "llvm/ADT/StringRef.h"
// using llvm::StringRef

#include "llvm/Support/Debug.h"
// using LLVM_DEBUG macro
// using llvm::dbgs

#define DEBUG_TYPE "oft-plugin-registration"

// plugin registration for opt new passmanager

namespace {

void parseModuleAnalyses(llvm::ModuleAnalysisManager &MAM) {
#define MODULE_ANALYSIS(NAME, CREATE_PASS)                                     \
  LLVM_DEBUG(llvm::dbgs() << "registering module analysis " << NAME << "\n";); \
  MAM.registerPass([]() { return CREATE_PASS; });

#include "Passes.def"

  return;
}

bool parseModulePipeline(llvm::StringRef Name, llvm::ModulePassManager &MPM,
                         llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
#define MODULE_ANALYSIS(NAME, CREATE_PASS)                                     \
  if (llvm::parseAnalysisUtilityPasses<                                        \
          std::remove_reference<decltype(CREATE_PASS)>::type>(NAME, Name,      \
                                                              MPM))            \
    return true;

#define MODULE_PASS(NAME, CREATE_PASS)                                         \
  if (Name == NAME) {                                                          \
    LLVM_DEBUG(llvm::dbgs() << "registering module pass " << NAME << "\n";);   \
    MPM.addPass(CREATE_PASS);                                                  \
    return true;                                                               \
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


