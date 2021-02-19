#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

#include <vector>

namespace oft {

struct LibraryScaleVariableDetectionInfo {
    const std::vector<llvm::Value *> scale_variables;
};

struct LibraryScaleVariableDetection {
    using Result = LibraryScaleVariableDetectionInfo;

    // TODO: LibSca... = default?;
    // explicit LibraryScaleVariableDetection() = default;

    std::vector<llvm::Value *> findMPIScaleVariables(llvm::Function *func);

    Result perform(llvm::Module &M, llvm::ModuleAnalysisManager &AM);
};

} // namespace oft
