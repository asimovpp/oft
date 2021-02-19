#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/User.h"

namespace oft {

struct OverflowInstrumentation
    : public llvm::PassInfoMixin<OverflowInstrumentation> {
    void instrumentInstruction(llvm::Instruction *I, unsigned int instr_id,
                               llvm::Function *instrumentFunc);

    void initInstrumentation(llvm::Module &M,
                             llvm::Function *initInstrumentFunc, int table_len);

    void finaliseInstrumentation(llvm::Module &M,
                                 llvm::Function *finaliseInstrumentFunc);

    llvm::Function *findFunction(llvm::Module &M, std::string funcName,
                                                llvm::Type *retTy,
                                                llvm::ArrayRef<llvm::Type *> argTys);

    llvm::PreservedAnalyses perform(llvm::Module &M,
                                    llvm::ModuleAnalysisManager &AM);
};

} // namespace oft
