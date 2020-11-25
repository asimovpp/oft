#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/IR/Instructions.h"

#include "llvm/IR/Module.h"

#include "llvm/IR/BasicBlock.h"

#include "llvm/IR/User.h"

#include "llvm/IR/Operator.h"

#include "llvm/Analysis/MemorySSA.h"

#include "OverflowTracking/ScaleGraph.hpp"

#include <unordered_set>

#include <map>

namespace oft {
    
    struct AnalyseScale : public llvm::PassInfoMixin<AnalyseScale> {
        // global variable to hold references to identified overflowable scale-dependent instructions  
        std::unordered_set<llvm::Instruction*> instr_to_instrument;  
        
        std::map<Function*, MemorySSA*> mssas;

        void printMemDefUseChain(llvm::Value* V, int i);

        void printMemUseDefChain(llvm::Value* V, int i);

        void dumpInstrAndMemorySSA(llvm::Function* func);

        bool canIntegerOverflow(llvm::Value* V);

        void instrumentInstruction(llvm::Instruction* I, unsigned int instr_id, llvm::Function* instrumentFunc);

        void initInstrumentation(llvm::Module& M, llvm::Function* initInstrumentFunc);

        void finaliseInstrumentation(llvm::Module& M, llvm::Function* finaliseInstrumentFunc);

        std::string getFunctionName(llvm::Instruction* inst);

        llvm::Function* findFunction(llvm::Module &M, std::string funcName);

        std::vector<llvm::Instruction*> getUsingInstr(llvm::StoreInst* storeInst);

        void printValue(llvm::Value* V, int depth);

        void followChain(llvm::Value* V, int depth, std::unordered_set<llvm::Value*> & visited);

        std::vector<llvm::Value*> findMPIScaleVariables(llvm::Function* func); 

        llvm::Value* findFirstDef(llvm::Value* v);

        scale_graph* createScaleGraph(std::vector<llvm::Value*> scale_variables); 
        
        void traceScaleInstructionsUpToCalls(llvm::Value* V, std::unordered_set<llvm::Value*> & visited, scale_graph* sg);
        
        std::vector<llvm::Value*> traceCallInstruction(llvm::Value* V, scale_graph* sg);
        
        void printTraces(llvm::Value* start, int depth, std::unordered_set<scale_node*> & visited, scale_graph* sg);
        
        void printTraces(scale_node* node, int depth, std::unordered_set<scale_node*> & visited);
        
        void findAndAddInstrToInstrument(scale_node* node, std::unordered_set<scale_node*> & visited);
        
        bool gepsAreEqual(llvm::GEPOperator* a, llvm::GEPOperator* b);
         
        void findGEPs(llvm::Value* V, std::vector<llvm::Value*>& geps);
        
        llvm::PreservedAnalyses track(llvm::Module &M, llvm::ModuleAnalysisManager &AM);
    
    };

}
