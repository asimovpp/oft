//
//
//

#include "OverflowTool/Analysis/ManualAnnotationSelection.hpp"

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/Debug.h"
// using LLVM_DEBUG macro
// using llvm::dbgs

#define DEBUG_TYPE "oft-manual-annotation-selection-analysis"

namespace oft {

ManualAnnotationSelection::ManualAnnotationSelection() {
    MarkedDB.emplace_back("scanf", false, std::vector<unsigned>{1u});
}

void ManualAnnotationSelection::visitCallInst(llvm::CallInst &CInst) {
    LLVM_DEBUG(llvm::dbgs()
                   << "processing call instruction: " << CInst << "\n";);
    auto *func = CInst.getCalledFunction();

    // if it is a Fortran function call, this should return the Function
    if (!func) {
        func = llvm::cast<llvm::Function>(
            CInst.getCalledValue()->stripPointerCasts());
    }

    if (func && func->getName() != ManualAnnotationFnName) {
        return;
    }

    assert(CInst.getNumArgOperands() == ManualAnnotationFnArgsNum &&
           "mismatched number of arguments in manual annotation function");

    visitDefaultAnnotationFunc(CInst);
}

void ManualAnnotationSelection::visitDefaultAnnotationFunc(
    llvm::CallInst &CInst) {
    auto *op0 = CInst.arg_begin()->get()->stripPointerCasts();

    // expect bitcast
    if (auto *bitcastInst = llvm::dyn_cast<llvm::BitCastInst>(op0)) {
        op0 = bitcastInst->getOperand(0);
    }

    Annotated.push_back(op0);
}

ManualAnnotationSelection::Result ManualAnnotationSelection::getAnnotated() {
    ManualAnnotationSelection::Result res;
    res.values.insert(Annotated.begin(), Annotated.end());

    return res;
}

} // namespace oft
