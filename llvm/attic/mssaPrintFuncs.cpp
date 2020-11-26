// .cpp

        // function to explore how MSSA works
        void AnalyseScale::printMemDefUseChain(Value* V, int i) {
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
        void AnalyseScale::printMemUseDefChain(Value* V, int i) {
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
        void AnalyseScale::dumpInstrAndMemorySSA(Function* func) {
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
        

// .hpp

        void printMemDefUseChain(llvm::Value* V, int i);

        void printMemUseDefChain(llvm::Value* V, int i);

        void dumpInstrAndMemorySSA(llvm::Function* func);
