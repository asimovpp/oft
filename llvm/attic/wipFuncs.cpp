        // I think a recursive depth-first(?) application will end up giving the single deepest definiton.
        // WIP
        /*
        Find the first definition of a variable. (if it is reused) (might not make sense)
        */
        Value* AnalyseScale::findFirstDef(Value* v) {
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
        

llvm::Value* findFirstDef(llvm::Value* v);
