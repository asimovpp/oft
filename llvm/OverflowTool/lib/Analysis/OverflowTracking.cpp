//===- OverflowTrackig.cpp - Testing integer overflow analysis ---------------===//
//
// No license at the moment.
// Justs Zarins
// j.zarins@epcc.ed.ac.uk
//
//===----------------------------------------------------------------------===//
//
// This class is for testing and learning on the way to integer overflow analysis.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/User.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/MemorySSA.h"

using namespace llvm;

#include <unordered_set>
#include <map>
#include <vector>
//#include "OverflowTracking.hpp"

namespace {
    struct scale_node {
        Value* value;
        bool could_overflow = false;
        std::vector<scale_node*> children; 
        std::vector<scale_node*> parents; 
        scale_node(Value* v) : value(v) {}
    };

    class scale_graph {
    public:
        typedef std::map<Value*, scale_node*> sgmap;
        sgmap graph;
        std::vector<scale_node*> scale_vars;
        void addvertex(Value*, bool);
        void addvertex(Value*, bool, bool);
        void addedge(Value* from, Value* to);
        scale_node* getvertex(Value*);
        void text_print();
        unsigned int get_size();
    };
    
    void scale_graph::addvertex(Value* val, bool is_root) {
        addvertex(val, is_root, false);
    }

    void scale_graph::addvertex(Value* val, bool is_root, bool could_overflow) {
        errs() << "Adding vertex " << *val << "\n";
        sgmap::iterator itr = graph.find(val);
        if (itr == graph.end()) {
            scale_node *v = new scale_node(val);
            v->could_overflow = could_overflow;
            if (is_root) scale_vars.push_back(v);
            graph[val] = v;
            return;
        }
        errs() << "\nVertex already exists!\n";
    }

    void scale_graph::addedge(Value* from, Value* to) {
        errs() << "Adding edge from " << *from << " to " << *to << "\n";
        sgmap::iterator itr_f = graph.find(from);
        sgmap::iterator itr_t = graph.find(to);
        if (itr_f != graph.end() && itr_t != graph.end()) {
            scale_node* f = (*itr_f).second;
            scale_node* t = (*itr_t).second;
            if (std::find(f->children.begin(), f->children.end(), t) == f->children.end()) {
                f->children.push_back(t);
                t->parents.push_back(f);
                return;
            } else {
                errs() << "\nEdge already exists!\n";
            }
        } else {
            errs() << "\nOne or both of the vertices are missing!\n";
        }
    }

    scale_node* scale_graph::getvertex(Value* val) {
        sgmap::iterator itr = graph.find(val);
        if (itr != graph.end()) {
            return itr->second;
        }
        return NULL;
    }

    void scale_graph::text_print() {
        errs() << "Printing scale graph\nScale variables:\n";
        for (scale_node* v : scale_vars) errs() << *(v->value) << "\n";
        errs() << "\nScale graph nodes:\n";
        for (auto& it : graph) {
            errs() << "val: " << *(it.second->value) << "\n";
            errs() << "owf: " << it.second->could_overflow << "\n";
            errs() << "chi: "; 
            for (scale_node* c : it.second->children) errs() << *(c->value) << "=+=";
            errs() << "\npar: ";
            for (scale_node* c : it.second->parents) errs() << *(c->value) << "=+=";
            errs() << "\n";
        }
    }

    unsigned int scale_graph::get_size() {
        return graph.size();
    }

    
    struct AnalyseScale : public ModulePass {
        static char ID; 
        AnalyseScale() : ModulePass(ID) {}

        // global variable to hold references to identified overflowable scale-dependent instructions  
        std::unordered_set<Instruction*> instr_to_instrument;  

        // function to explore how MSSA works
        void printMemDefUseChain(Value* V, int i) {
            if (Instruction* Inst = dyn_cast<Instruction>(V)) {
                Function* caller = Inst->getParent()->getParent();
                MemorySSA &mssa = getAnalysis<MemorySSAWrapperPass>(*caller).getMSSA();
                MemoryUseOrDef *mem = mssa.getMemoryAccess(&*Inst);
                if (mem) {
                    errs() << i << " ||| " << *(mem->getMemoryInst()) << " ||| " << *mem << "\n";
                    for (User *U : mem->users()) {
                        printMemDefUseChain(U, i+1); 
                    }
                }
            } else if (MemoryUseOrDef* mem = dyn_cast<MemoryUseOrDef>(V)) {
                errs() << i << " ||| " << *(mem->getMemoryInst()) << " ||| " << *mem << "\n";
                for (User *U : mem->users()) {
                    printMemDefUseChain(U, i+1); 
                }
            }
        }


        // function to explore how MSSA works
        void printMemUseDefChain(Value* V, int i) {
            if (Instruction* Inst = dyn_cast<Instruction>(V)) {
                Function* caller = Inst->getParent()->getParent();
                MemorySSA &mssa = getAnalysis<MemorySSAWrapperPass>(*caller).getMSSA();
                MemoryUseOrDef *mem = mssa.getMemoryAccess(&*Inst);
                if (mem) {
                    errs() << i << " ||| " << *(mem->getMemoryInst()) << " ||| " << *mem << "\n";
                    for (Use &U : mem->operands()) {
                        printMemUseDefChain(U.get(), i-1); 
                    }
                } else {
                    errs() << i << " ||| " << "no Instruction" << " ||| " << *Inst << "\n";
                }
            } else if (MemoryUseOrDef* mem = dyn_cast<MemoryUseOrDef>(V)) {
                if (mem->getMemoryInst()) {
                    errs() << i << " ||| " << *(mem->getMemoryInst()) << " ||| " << *mem << "\n";
                    for (Use &U : mem->operands()) {
                        printMemUseDefChain(U.get(), i-1); 
                    }
                } else {
                    errs() << i << " ||| " << "no Instruction" << " ||| " << *mem << "\n";
                }
            }
        }


        // function to explore how MSSA works
        void dumpInstrAndMemorySSA(Function* func) {
            if (! func->isDeclaration()) {
                MemorySSA &mssa = getAnalysis<MemorySSAWrapperPass>(*func).getMSSA();
                for (inst_iterator I = inst_begin(*func), e = inst_end(*func); I != e; ++I) {
                    int line_num = -1;
                    if (I->getDebugLoc())
                        line_num = I->getDebugLoc().getLine();

                    MemoryUseOrDef *mem = mssa.getMemoryAccess(&*I);
                    if (mem) {
                        errs() << *I << "\t||| " << *mem << "\t||| on source Line " << line_num << "\n";
                    } else {
                        errs() << *I << "\t||| " << "no MemSSA" << "\t||| on source Line " << line_num << "\n";
                    }
                    printMemDefUseChain(&*I, 0);
                } 
            }
        }


/*==============================================================================================================================*/

        /*
        Check that it is the right type of instruction and
        that it is one of the instructions we care about
        i.e. an arithmetic function operating on an integer 32 bits in size or smaller.
        */
        bool canIntegerOverflow(Value* V) {
            const std::unordered_set<unsigned> overflow_ops = {Instruction::Add, Instruction::Sub, Instruction::Mul, Instruction::Shl, Instruction::LShr, Instruction::AShr};  
            //TODO: what would happen if the operation was between 32 bit and 64 bit values? would the needed cast be in a separate instrucion somewhere?
            if (BinaryOperator* I = dyn_cast<BinaryOperator>(V)) {
                if (overflow_ops.find(I->getOpcode()) != overflow_ops.end() &&
                        I->getType()->isIntegerTy() &&
                        I->getType()->getScalarSizeInBits() <= 32) {
                    errs() << "     Instruction " << *I << " could overflow. Has type " << *(I->getType()) << "\n"; 
                    return true;
                }
            }
            return false;
        }


        /*
        Insert instrumentation call for instruction I. 
        instr_id is passed to the instrumentation call to differentiate between instrumented results 
        in the output from the instrumented application.
        */
        void instrumentInstruction(Instruction* I, unsigned int instr_id, Function* instrumentFunc) {
            // see : https://stackoverflow.com/questions/51082081/llvm-pass-to-insert-an-external-function-call-to-llvm-bitcode
            //ArrayRef< Value* > arguments(ConstantInt::get(Type::getInt32Ty(I->getContext()), I, true));

            Value* counterVal = llvm::ConstantInt::get(I->getContext(), llvm::APInt(32, instr_id, true));
            std::vector<Value*> args = {counterVal, I};
            errs() << "ID " << instr_id << " given to "; 
            printValue(I, 0);
            ArrayRef< Value* > argRef(args);
            //errs() << "Inserting func with type " << *(instrumentFunc->getFunctionType()) << "\n";
            //errs() << "Func is " << *(instrumentFunc) << "\n";
            Instruction* newInst = CallInst::Create(instrumentFunc, argRef, "");
            
            //auto* newInst = new CallInst(instrumentFunc, I, "overflowInstrumentation", I);
            //Instruction *newInst = CallInst::Create(instrumentFunc, I, "");
            //Instruction *newInst = new CallInst(instrumentFunc, I, "");
            //I->getParent()->getInstList().insertAfter(I, newInst);
            newInst->insertAfter(I);


            //Type *PtrTy = PointerType::getUnqual(Type::Int64Ty);
            //CastInst *CI = CastInst::Create(Instruction::BitCast, I, PtrTy, "");
            //newInst->insertAfter(CI);
        }

        
        /*
        Insert instrumentation initialisation at the start of the main function.
        */
        void initInstrumentation(Module& M, Function* initInstrumentFunc) {
            for (Module::iterator func = M.begin(), e = M.end(); func != e; ++func) {
                if (func->getName() == "main" || func->getName() == "MAIN_") {
                    errs() << "Inserting instrumentation initialisation\n";

                    std::vector<Value*> args = {};
                    ArrayRef< Value* > argRef(args);
                    Instruction* newInst = CallInst::Create(initInstrumentFunc, argRef, "");
                    BasicBlock& BB = func->getEntryBlock();
                    Instruction* I = BB.getFirstNonPHIOrDbg(); 
                    newInst->insertBefore(I);

                    break; 
                }
            }
        }


        /*
        Insert instrumentation finalisation before a call to mpi_finalize.
        It is assumed that this occurs near the exit of the application and that mpi_finalize is called only once.
        */
        void finaliseInstrumentation(Module& M, Function* finaliseInstrumentFunc) {
            const std::unordered_set<std::string> mpi_finalize_functions = {"MPI_Finalize", "mpi_finalize_", "mpi_finalize_f08_"};

            for (Module::iterator func = M.begin(), e = M.end(); func != e; ++func) {
                //if (func->getName() == "main" || func->getName() == "MAIN_") 
                if (true) {
                    for (inst_iterator I = inst_begin(*func), e = inst_end(*func); I != e; ++I) {
                        //if (isa<ReturnInst>(&*I)) 
                        if (isa<CallInst>(&*I) && mpi_finalize_functions.find(getFunctionName(&*I)) != mpi_finalize_functions.end()) {
                            std::vector<Value*> args = {};
                            ArrayRef< Value* > argRef(args);
                            Instruction* newInst = CallInst::Create(finaliseInstrumentFunc, argRef, "");
                            errs() << "Inserting instrumentation finalisation before line " << I->getDebugLoc()->getLine() << " in file " << I->getDebugLoc()->getFilename() <<  "\n";
                            newInst->insertBefore(&*I);
                        }
                    }

                    //break; 
                }
            }
        }

        
        /*
        Find the function name coming from a call instruction.
        Has special cases C and Fortran LLVM IR.
        */
        std::string getFunctionName(Instruction* inst) {
            std::string func_name = "";
            if (auto *callInst = dyn_cast<CallInst>(inst)) {
                // getCalledFunction() Returns the function called, or null if this is an indirect function invocation. 
                Function* fp =  callInst->getCalledFunction();

                if (fp == NULL) {
                    // Fortran LLVM IR does some bitcast on every function before calling it, thus losing information about the original call.
                    //func_name = callInst->getCalledOperand()->stripPointerCasts()->getName().str();
                    func_name = callInst->getCalledValue()->stripPointerCasts()->getName().str();
                } else {
                    // in LLVM IR from C it is straightforward to get the function name
                    func_name = callInst->getCalledFunction()->getName().str();
                }
            }
            return func_name;
        }

        
        /*
        Find the function pointer by name in the given module.
        */
        Function* findFunction(Module &M, std::string funcName) {
            Function* out = NULL;
            for (Module::iterator func = M.begin(), e = M.end(); func != e; ++func) {
                if (func->getName() == funcName) {
                    errs() << "Found " << funcName << " function\n";
                    out = &*func;
                    break;
                }
            }
            return out;
        }


        /*
        Use MemSSA to find load instructions corresponding to a store instruction.
        */
        std::vector<Instruction*> getUsingInstr(StoreInst* storeInst) {
            std::vector<Instruction*> out;

            //parent of instruction is basic block, parent of basic block is function (?)
            Function* caller = storeInst->getParent()->getParent();
            errs() << "store inst functin is " << caller->getName() << "\n";
            MemorySSA &mssa = getAnalysis<MemorySSAWrapperPass>(*caller).getMSSA();
            MemoryUseOrDef *mem = mssa.getMemoryAccess(&*storeInst);
            if (mem) {
                errs() << *mem << "\n";
                for (User* U : mem->users()) {
                    if (MemoryUse *m = dyn_cast<MemoryUse>(U)) {
                        errs() << "user " << *m << "\n";
                        Instruction *memInst = m->getMemoryInst();
                        errs() << "user inst " << *memInst << "\n";
                        if (isa<LoadInst>(memInst)) {
                            out.push_back(memInst);
                        }
                    }
                }
            }

            return out;
        }


        /*
        Print an instruction along with some of its debug information.
        Depth controls the indentation of the printed line.
        */
        void printValue(Value* V, int depth) {
            //if (depth == 0) {
            //    errs() << *V <<"\n";
            //} 
            int line_num = -1;
            StringRef fileName = "unknown";
                
            if (Instruction *Inst = dyn_cast<Instruction>(V)) {
                DILocation* loc = Inst->getDebugLoc();
                if (loc) {
                    line_num = loc->getLine();
                    fileName = loc->getFilename();
                }
            } 
            
            errs() << "├";
            for (int i = 0; i < depth; ++i)
                errs() << "-";
            errs() << *V << " on Line " << line_num << " in file " << fileName << "\n";
        }
       
        
        /*
        Create a scale graph based on the provided list of scale variables (starting points to tracing).
        */
        scale_graph* createScaleGraph(std::vector<Value*> scale_variables) {
            scale_graph* sg = new scale_graph;

            for (Value* scale_var : scale_variables) sg->addvertex(scale_var, true);
            
            // Iterate through scale variables and find all instructions which they influence (scale instructions)
            for (Value* V : scale_variables) {
                std::unordered_set<Value*> visited; 
                if (isa<AllocaInst>(V)) {
                    errs() << "tracing scale variable (alloca): " << *V << "\n"; 
                    traceScaleInstructionsUpToCalls(V, visited, sg);
                } else if (isa<GlobalVariable>(V)) { 
                    errs() << "tracing scale variable (global): " << *V << "\n"; 
                    traceScaleInstructionsUpToCalls(V, visited, sg);
                } else if (auto* gep = dyn_cast<GEPOperator>(V)) { 
                    errs() << "tracing scale variable (GEP): " << *V << "\n"; 
                    if (GlobalVariable* gv = dyn_cast<GlobalVariable>(gep->getPointerOperand()->stripPointerCasts())) {
                        //errs() << "xxx| gv = " << *gv << "\n";
                        std::vector<Value*> geps;
                        findGEPs(gv, geps);
                        for (auto* GVgep : geps) {
                            //errs() << "xxx| GVgep = " << *GVgep << "\n";
                            if (gepsAreEqual(cast<GEPOperator>(gep), cast<GEPOperator>(GVgep))) {
                                //need to connect the equivalent gep (UUgep) to the original scale variable (gep) in order to make the scale graph sensible.
                                //TODO: Is there a better way of doing this?
                                sg->addvertex(GVgep, false);
                                sg->addedge(gep, GVgep);
                                traceScaleInstructionsUpToCalls(GVgep, visited, sg);
                            }
                        }
                    } else {
                        errs() << "No rule for tracing global variable that isn't a struct " << *gv << " coming from " << *V << "\n";
                    }
                } else {
                    errs() << "No rule for tracing value " << *V << "\n";
                }
            }
           
            errs() << "tracing call sites\n";
            //expand call instructions iteratively until no more changes occur 
            unsigned int prevSize = 0;
            while (sg->get_size() - prevSize != 0) {
                std::vector<Value*> to_follow; 
                for (auto& node : sg->graph) {
                   //for (scale_node* c : it.second->parents) errs() << *(c->value) << "=+=";
                    if (isa<CallInst>(node.first)) {
                        //errs() << "found call site " << *(node.first) << "\n";
                        std::vector<Value*> continuations = traceCallInstruction(node.first, sg);
                        to_follow.insert(to_follow.end(), continuations.begin(), continuations.end());
                    }
                }
            
                for (Value* child : to_follow) {
                    //errs() << "following call site via " << *(child) << "\n";
                    std::unordered_set<Value*> visited; 
                    traceScaleInstructionsUpToCalls(child, visited, sg);
                }
                
                prevSize = sg->get_size();
            }
       
            return sg;
        }
        

        /*
        Connect a call site to the called function's body via argument positions in the scale graph.
        */
        std::vector<Value*> traceCallInstruction(Value* V, scale_graph* sg) {
            std::vector<Value*> children;
            
            //the tracing is continued across function calls through argument position
            if (CallInst* callInst = dyn_cast<CallInst>(V)) { //TODO: also check if the number of users is =0?
                for (scale_node* parent : sg->getvertex(V)->parents) {
                    //errs() << "checking " << *callInst << " and user " << *V << "\n"; 
                    for (unsigned int i = 0; i < callInst->getNumOperands(); ++i) {
                        //errs() << "checking " << i << "th operand which is " << *(callInst->getOperand(i)) << "\n"; 
                        if (callInst->getOperand(i) == parent->value) {
                            Function* fp = callInst->getCalledFunction();
                            if (fp == NULL)
                                fp = dyn_cast<Function>(callInst->getCalledValue()->stripPointerCasts());
                            //errs() << "V is " << i << "th operand of " << *callInst << "; Function is " << fp->getName() << "\n";
                            if (! fp->isDeclaration()) { //TODO: check number of arguments; some are variadic
                                //errs() << "     Tracing in function body of called function via " << i << "th argument. ( " << fp->getName()  << " )\n";
                                //followChain(fp->getArg(i), depth+1, visited);
                                Value* arg_to_track = &*(fp->arg_begin() + i);
                                children.push_back(arg_to_track);
                                sg->addvertex(arg_to_track, false);
                                sg->addedge(V, arg_to_track);
                            } else {
                                //errs() << "     Function body not available for further tracing. ( " << fp->getName()  << " )\n";
                            }
                        }
                    }
                }
            }
            
            return children;
        }
        
        
        /*
        Trace scale variable (arg 1) and add visited nodes to scale graph.
        Stop at call sites.
        Already visited nodes are skipped the second time.
        */
        void traceScaleInstructionsUpToCalls(Value* V, std::unordered_set<Value*> & visited, scale_graph* sg) {
            errs() << "Visiting node " << visited.size() << "\t" << *V << "\n"; 
            if (visited.find(V) != visited.end()) {
                errs() << "Node " << *V << " has already been visited. Skipping.\n";
                return;
            }
            visited.insert(V);

            std::vector<Value*> children;
            for (User* U : V->users()) {
                children.push_back(U);
                sg->addvertex(U, false);
                sg->addedge(V, U);
            }

            //store instructions require MemSSA to connect them to their corresponding load instructions in the chain
            if (StoreInst* storeInst = dyn_cast<StoreInst>(V)) { //TODO: also check if the number of users is =0?
                std::vector<Instruction*> memUses = getUsingInstr(storeInst);
                children.insert(children.end(), memUses.begin(), memUses.end());
                for (Instruction* I : memUses) {
                    children.push_back(I);
                    sg->addvertex(I, false);
                    sg->addedge(V, I);
                }
            } 

            for (Value* child : children) {
                traceScaleInstructionsUpToCalls(child, visited, sg);
            }
        }



        /*
        Pretty print scale graph starting from "start".
        */
        void printTraces(Value* start, int depth, std::unordered_set<scale_node*> & visited, scale_graph* sg) {
            printTraces(sg->getvertex(start), depth, visited);
        }

        void printTraces(scale_node* node, int depth, std::unordered_set<scale_node*> & visited) {
            if (visited.find(node) != visited.end()) {
                errs() << "Node " << *(node->value) << " already visited\n";
                return;
            }
            visited.insert(node);
            printValue(node->value, depth);
            for (scale_node* n : node->children) printTraces(n, depth+1, visited);
        }


        /*
        Traverse scale graph starting from "node", tag instructions that can overflow and add them to list of to-be-instrumented-instructions.
        */
        void findAndAddInstrToInstrument(scale_node* node, std::unordered_set<scale_node*> & visited) {
            if (visited.find(node) != visited.end()) return;
            visited.insert(node);
            //check each visited node whether it should be instrumented and add to a list if it should be
            if (canIntegerOverflow(node->value)) {
                instr_to_instrument.insert(cast<Instruction>(node->value));
                node->could_overflow = true;
            }
            for (scale_node* n : node->children) findAndAddInstrToInstrument(n, visited);
        }

        
        /*
        Find scale variables that are initiated as a mpi_comm_rank or mpi_comm_size variable.
        */
        std::vector<Value*> findMPIScaleVariables(Function* func) { 
            // List of scale functions in MPI. Names as found in C and Fortran.
            const std::unordered_set<std::string> mpi_scale_functions = {"MPI_Comm_size", "MPI_Comm_rank", "MPI_Group_size", "MPI_Group_rank", "mpi_comm_size_", "mpi_comm_rank_", "mpi_group_size_", "mpi_group_rank_", "mpi_comm_size_f08_", "mpi_comm_rank_f08_", "mpi_group_size_f08_", "mpi_group_rank_f08_"};  
            std::vector<Value*> vars;

            // Iterate through all instructions and find the MPI rank/size calls
            // Then store pointers to the corresponding scale variables
            for (inst_iterator I = inst_begin(*func), e = inst_end(*func); I != e; ++I) {
                // TODO: might have to use CallSite wrapper instead to also catch "function invokation"
                if (auto *callInst = dyn_cast<CallInst>(&*I)) {
                    if (mpi_scale_functions.find(getFunctionName(callInst)) != mpi_scale_functions.end()) {
                        // the scale variable is always the 2nd operand in the MPI functions of interest
                        Value* scale_var = callInst->getOperand(1)->stripPointerCasts();
                        //Value* firstDef = findFirstDef(scale_var);
                        //errs() << *scale_var << " oldest ref is: " << *firstDef << "\n"; 

                        if (isa<AllocaInst>(scale_var)) {
                            errs() << *I << " sets scale variable (alloca): " << *scale_var << "\n"; 
                            vars.push_back(scale_var);
                        } else if (isa<GlobalVariable>(scale_var)) { 
                            errs() << *I << " sets scale variable (global): " << *scale_var << "\n"; 
                            vars.push_back(scale_var);
                        } else if (auto* gep = dyn_cast<GEPOperator>(scale_var)) { 
                            errs() << *I << " sets scale variable (GEP): " << *scale_var << "\n"; 
                            vars.push_back(gep);
                        } else {
                            errs() << *scale_var << " is not alloca or global" << "\n"; 
                            errs() << *callInst << " is the scale function that was called" << "\n"; 
                        }
                    }
                }
            }

            return vars;
        }


        /*
        Test whether two GEP calls *look* the same,
        i.e. the same target and offsets.
        What the results of the GEP call would be in practice is not considered.
        */
        bool gepsAreEqual(GEPOperator* a, GEPOperator* b) {
            //errs() << "     Comparing " << *a << " and " << *b; 

            bool areEqual = true;
            areEqual = areEqual && a->getSourceElementType() == b->getSourceElementType();
            areEqual = areEqual && a->getPointerOperandType() == b->getPointerOperandType();
            areEqual = areEqual && a->getPointerOperand()->stripPointerCasts() == b->getPointerOperand()->stripPointerCasts();
            areEqual = areEqual && a->getNumIndices() == b->getNumIndices();
            if (areEqual) {
                // iterate over both sets if indices simultaneously (they are same length)
                for (auto aIter = a->idx_begin(), bIter = b->idx_begin(),
                        aEnd = a->idx_end(), bEnd = b->idx_end();
                        aIter != aEnd || bIter !=bEnd; 
                        ++aIter, ++bIter) {
                    areEqual = areEqual && *aIter == *bIter;
                    //errs() << "\n===| " << *aIter << " and " << *bIter << "\n"; 
                }
            }
            
            
            /*if (areEqual) {
                errs() << "; they are equal\n";
            } else {
            
                errs() << "; not equal\n";
            }*/


            //things not checked: 
            //a->getResultElementType() 
            //a->getAddressSpace() 
            //a->getPointerAddressSpace() 

            return areEqual; 
        }


        /*
        Unpick bitcasts etc. to find the root GEP instruction. (might not be generalisable) 
        TODO: can one encounter loops in this search of the graph?
        */
         void findGEPs(Value* V, std::vector<Value*>& geps) {
            //errs() << "ooo| finding GEPs for " << *V << "\n"; 
            for (User *U : V->users()) {
                //errs() << "ooo| U = " << *U << "\n"; 
                //if (auto* Ugep = dyn_cast<GEPOperator>(U->stripPointerCasts())) {
                if (auto* Ugep = dyn_cast<GEPOperator>(U)) {
                    //errs() << "ooo| it's a gep" << "\n"; 
                    geps.push_back(Ugep); //found a gep, stop searching on this branch
                } else {
                    //errs() << "ooo| it's NOT a gep " << "\n"; 
                    findGEPs(U, geps); //otherwise, look another level down
                }
            }
        } 


        // I think a recursive depth-first(?) application will end up giving the single deepest definiton.
        // WIP
        /*
        Find the first definition of a variable. (if it is reused) (might not make sense)
        */
        Value* findFirstDef(Value* v) {
            errs() << "checking " << *v << "\n"; 
            Value* out = v;
            if (Instruction *scI = dyn_cast<Instruction>(v)) {
                if (scI->getNumOperands() != 0) {
                    for (Use &U : scI->operands()) {
                        Value *next_v = U.get();
                        out = findFirstDef(next_v);
                    }
                }
            } else {
                errs() << "ignoring " << *v << " becuse not an Instruction\n"; 
            }

            return out;
        } 


/*==============================================================================================================================*/
        bool runOnModule(Module &M) override {
           

            //Find scale variables in this module
            std::vector<Value*> scale_variables;
            for (Module::iterator func = M.begin(), e = M.end(); func != e; ++func) {
                errs() << "Looking for scale variables in Function: " << func->getName() << "\n"; 
            
                const std::unordered_set<std::string> functions_to_ignore = {"store_max_val", "init_vals", "print_max_vals"};
                if (functions_to_ignore.find(func->getName()) == functions_to_ignore.end()) {
                    std::vector<Value*> func_scale_vars = findMPIScaleVariables(&*func);
                    scale_variables.insert(scale_variables.end(), func_scale_vars.begin(), func_scale_vars.end());
                }

                //dumpInstrAndMemorySSA(&*func);
            }

            errs() << "\n--------------------------------------------\n"; 
            errs() << "Scale variables found:\n"; 
            for (Value* V : scale_variables) {
                printValue(V, 0);
            }
            errs() << "--------------------------------------------\n"; 

            scale_graph* sg = createScaleGraph(scale_variables);

            errs() << "--------------------------------------------\n"; 
            
            errs() << "\nPrinting scale variable def-use chains\n"; 
            for (scale_node* v : sg->scale_vars) {
                std::unordered_set<scale_node*> visited;
                printTraces(v, 0, visited);
            }
                 
        
            errs() << "--------------------------------------------\n"; 
            
            for (scale_node* v : sg->scale_vars) {
                std::unordered_set<scale_node*> visited;
                findAndAddInstrToInstrument(v, visited);
            }
            errs() << "--------------------------------------------\n"; 
            //insert instrumentation after scale instructions, plus setup/teardown calls for the instrumentation
            Function* instrumentFunc = findFunction(M, "store_max_val");
            unsigned int instr_id = 0;
            for (Instruction* I : instr_to_instrument) {
                instrumentInstruction(I, instr_id, instrumentFunc);
                instr_id++;
            }
            errs() << "--------------------------------------------\n"; 

            initInstrumentation(M, findFunction(M, "init_vals"));
            finaliseInstrumentation(M, findFunction(M, "print_max_vals"));
            errs() << "--------------------------------------------\n"; 

            sg->text_print();

            // return true/false depending on whether the pass did/didn't change the input IR
            return true;
        }


        //specify other passes that this pass depends on
        void getAnalysisUsage(AnalysisUsage &AU) const override {
            // (should this be addRequiredTransitive instead?)
            //AU.addRequired<DependenceAnalysisWrapperPass>();
            AU.addRequired<MemorySSAWrapperPass>();
            // this pass doesn't invalidate any subsequent passes
            // AU.setPreservesAll();
        }




    };
}

char AnalyseScale::ID = 0;
static RegisterPass<AnalyseScale> X("analyse_scale", "Analyse application scale variables");