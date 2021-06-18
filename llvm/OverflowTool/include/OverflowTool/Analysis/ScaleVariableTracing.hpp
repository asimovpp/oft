#pragma once

#include "OverflowTool/Config.hpp"
#include "OverflowTool/ScaleGraph.hpp"

#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/PassManager.h"

#include <unordered_set>
#include <vector>

namespace llvm {
class Module;
class Function;
class Value;
class GEPOperator;
class StoreInst;
} // namespace llvm

namespace oft {

struct ScaleVariableTracingInfo {
    scale_graph graph;
};

struct ValueTrace {
    std::vector<Value *> trace;
    ValueTrace(Value *v) {addValue(v);} 
    ValueTrace(ValueTrace *vt) {trace = vt->trace;} 

    void addValue(Value *V) {trace.push_back(V);}
    Value* getHead() {return trace.front();}
    Value* getTail() {return trace.back();}
};

struct ScaleVariableTracing {
    std::map<Function *, MemorySSA *> mssas;

    using Result = ScaleVariableTracingInfo;

    scale_graph *createScaleGraph(std::vector<llvm::Value *> scale_variables);

    void
    traceScaleInstructionsUpToCalls(llvm::Value *V,
                                    std::unordered_set<llvm::Value *> &visited,
                                    scale_graph *sg);

    std::vector<llvm::Value *> traceCallInstruction(llvm::Value *V,
                                                    scale_graph *sg);

    std::vector<llvm::Instruction *> getUsingInstr(llvm::StoreInst *storeInst);
    
    SmallVector<ValueTrace *, 8> followBwd(ValueTrace *vt);

    ValueTrace *followBwdUpToArg(ValueTrace *V);
    
    SmallVector<Value *, 8> followArg(Value *V);

    void analyseTrace(ValueTrace *vt);

    Instruction *getStore(LoadInst *loadInst);

    void findGEPs(llvm::Value *V, std::vector<llvm::Value *> &geps);

    bool gepsAreEqual(llvm::GEPOperator *a, llvm::GEPOperator *b);

    Result perform(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);
};

} // namespace oft
