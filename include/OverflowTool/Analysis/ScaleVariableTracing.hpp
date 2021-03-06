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
class LoopInfo;
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
    unsigned getSize() {return trace.size();}
    std::vector<Value *>::iterator begin() {return trace.begin();}    
    std::vector<Value *>::iterator end() {return trace.end();}    
    std::vector<Value *>::reverse_iterator rbegin() {return trace.rbegin();}    
    std::vector<Value *>::reverse_iterator rend() {return trace.rend();}    
};

struct ScaleVariableTracing {
    std::map<Function *, MemorySSA *> mssas;
    std::map<Function *, LoopInfo *> lis;

    using Result = ScaleVariableTracingInfo;

    scale_graph *
    createScaleGraph(const std::vector<Value *> &scale_variables);

    void trace(std::vector<llvm::Value *> scale_variables, scale_graph &sg);

    void
    traceScaleInstructionsUpToCalls(llvm::Value *V,
                                    std::unordered_set<llvm::Value *> &visited,
                                    scale_graph &sg);

    std::vector<Value *> traceCallInstruction(Value *V,
                                                    scale_graph *sg);
    std::vector<llvm::Value *> traceCallInstruction(llvm::Value *V,
                                                    scale_graph &sg);

    std::vector<Value *> getCallArgs(CallInst *callInst, Value* arg);

    std::vector<Instruction *> getUsingInstr(StoreInst *storeInst);
    
    SmallVector<ValueTrace *, 8> followBwd(ValueTrace *vt);

    ValueTrace *followBwdUpToArg(ValueTrace *V);

    void followArithmeticBwd(ValueTrace *vt);

    bool indicesAreEqual(Value *a, Value *b);
    
    SmallVector<Value *, 8> followArg(Value *V);

    SmallVector<Value *, 8> analyseTrace(ValueTrace *vt);

    Value *getStore(LoadInst *loadInst);

    bool naiveCompare(Value *a, Value *b);

    void findGEPs(Value *V, std::vector<Value *> &geps, std::unordered_set<Value *> &visited);

    bool gepsAreEqual(GEPOperator *a, GEPOperator *b);

    bool gepsAreEqual(GEPOperator *a, GEPOperator *b, Value *rootA, Value *rootB);

    Result perform(llvm::Module &M, llvm::ModuleAnalysisManager &MAM,
                   bool shouldTraceLoops = false);

    void traceLoops(scale_graph &sg);
};

} // namespace oft
