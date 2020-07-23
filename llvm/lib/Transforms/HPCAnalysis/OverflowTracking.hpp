#ifndef OVERFLOWTRACKING_HPP
#define OVERFLOWTRACKING_HPP

namespace {
    // global variable to hold references to identified overflowable scale-dependent instructions  
    std::unordered_set<Instruction*> instr_to_instrument;  

    void printMemDefUseChain(Value* V, int i);

    void printMemUseDefChain(Value* V, int i);

    void dumpInstrAndMemorySSA(Function* func);

    bool canIntegerOverflow(Value* V);

    void instrumentInstruction(Instruction* I, unsigned int instr_id, Function* instrumentFunc);

    void initInstrumentation(Module& M, Function* initInstrumentFunc);

    void finaliseInstrumentation(Module& M, Function* finaliseInstrumentFunc);

    std::string getFunctionName(Instruction* inst);

    Function* findFunction(Module &M, std::string funcName);

    std::vector<Instruction*> getUsingInstr(StoreInst* storeInst);

    void printValue(Value* V, int depth);

    void followChain(Value* V, int depth, std::set<Value*> & visited);

    std::vector<Value*> findMPIScaleVariables(Function* func); 

    bool gepsAreEqual(GetElementPtrInst* a, GetElementPtrInst* b);

    void findGEPs(Value* V, int depth);

    Value* findFirstDef(Value* v);
}

#endif // OVERFLOWTRACKING_HPP
