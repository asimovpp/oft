#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/IR/Module.h"

#include "llvm/IR/BasicBlock.h"

#include "llvm/IR/User.h"

#include "llvm/Analysis/MemorySSA.h"

namespace oft {
    
    struct AnalyseScale : public llvm::PassInfoMixin<AnalyseScale> {
        void instrumentInstruction(llvm::Instruction* I, unsigned int instr_id, llvm::Function* instrumentFunc);

        void initInstrumentation(llvm::Module& M, llvm::Function* initInstrumentFunc);

        void finaliseInstrumentation(llvm::Module& M, llvm::Function* finaliseInstrumentFunc);

        llvm::Function* findFunction(llvm::Module &M, std::string funcName);
        
        llvm::PreservedAnalyses track(llvm::Module &M, llvm::ModuleAnalysisManager &AM);
    };

}
