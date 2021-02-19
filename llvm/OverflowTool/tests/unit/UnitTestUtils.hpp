//
//
//

#pragma once

#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include <memory>
#include <string>

inline auto parseModule(const std::string ModuleStr, llvm::LLVMContext &Ctx) {
    llvm::SMDiagnostic err;
    return llvm::parseAssemblyString(ModuleStr, err, Ctx);
}
