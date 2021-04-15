#pragma once

#include "OverflowTool/Config.hpp"
#include "OverflowTool/ScaleGraph.hpp"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

namespace llvm {
class Module;
class Function;
class Instruction;
class Type;
} // namespace llvm

namespace oft {

struct OverflowInstrumentation {
    OverflowInstrumentation(llvm::Module &M) : M(M) {}

    void instrumentInstruction(llvm::Instruction *I, unsigned int instr_id,
                               llvm::Function *instrumentFunc);

    void initInstrumentation(llvm::Module &M,
                             llvm::Function *initInstrumentFunc, int table_len);

    void finaliseInstrumentation(llvm::Module &M,
                                 llvm::Function *finaliseInstrumentFunc);

    llvm::Function *findFunction(llvm::Module &M, std::string funcName,
                                 llvm::Type *retTy,
                                 llvm::ArrayRef<llvm::Type *> argTys);

    void instrument(llvm::ArrayRef<llvm::Instruction *> Overflowable);

  private:
    llvm::Module &M;
};

} // namespace oft
