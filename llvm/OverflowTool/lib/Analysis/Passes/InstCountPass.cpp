#include "OverflowTool/Analysis/Passes/InstCountPass.hpp"

#include "OverflowTool/Config.hpp"
#include "OverflowTool/Debug.hpp"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

#include <set>

#define DEBUG_TYPE OFT_INSTCOUNT_PASS_NAME

STATISTIC(TotalIntArithmeticInstructions,
          "Number of 32-bit integer arithmetic instructions");

namespace oft {

struct InstCount : public llvm::InstVisitor<InstCount> {
    friend class llvm::InstVisitor<InstCount>;

    const std::set<unsigned> ArithOps = {
        llvm::Instruction::Add,  llvm::Instruction::Sub,
        llvm::Instruction::Mul,  llvm::Instruction::Shl,
        llvm::Instruction::LShr, llvm::Instruction::AShr};

    const unsigned IntArithmeticOperandBits = 32;

    void visitBinaryOperator(llvm::BinaryOperator &Op) {
        if (ArithOps.count(Op.getOpcode()) && Op.getType()->isIntegerTy() &&
            Op.getType()->getScalarSizeInBits() <= IntArithmeticOperandBits) {
            ++TotalIntArithmeticInstructions;
        }
    }
};

llvm::PreservedAnalyses InstCountPass::run(llvm::Module &M,
                                           llvm::ModuleAnalysisManager &MAM) {
    OFT_DEBUG(llvm::dbgs() << "OFT INSTCOUNT: running on module: "
                           << M.getName() << "\n");
    InstCount().visit(M);
    OFT_DEBUG(
        llvm::dbgs() << "Number of 32-bit integer arithmetic instructions: "
                     << TotalIntArithmeticInstructions << "\n";);

    return llvm::PreservedAnalyses::all();
}

} // namespace oft
