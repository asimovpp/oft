#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/IR/Module.h"

#include "llvm/IR/BasicBlock.h"

#include "llvm/IR/User.h"

#include "llvm/Analysis/MemorySSA.h"

#include "OverflowTracking/ScaleGraph.hpp"

#include <unordered_set>

#include <map>

namespace oft {
    
    struct AnalyseScale : public llvm::PassInfoMixin<AnalyseScale> {
        // global variable to hold references to identified overflowable scale-dependent instructions  
        std::unordered_set<llvm::Instruction*> instr_to_instrument;  
        
        bool canIntegerOverflow(llvm::Value* V);

        void instrumentInstruction(llvm::Instruction* I, unsigned int instr_id, llvm::Function* instrumentFunc);

        void initInstrumentation(llvm::Module& M, llvm::Function* initInstrumentFunc);

        void finaliseInstrumentation(llvm::Module& M, llvm::Function* finaliseInstrumentFunc);

        llvm::Function* findFunction(llvm::Module &M, std::string funcName);

        void findAndAddInstrToInstrument(scale_node* node, std::unordered_set<scale_node*> & visited);
        
        llvm::PreservedAnalyses track(llvm::Module &M, llvm::ModuleAnalysisManager &AM);
    
    };

}
