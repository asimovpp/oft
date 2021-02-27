//

#include "OverflowTool/Analysis/ManualAnnotationSelection.hpp"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"

#include <algorithm>

#define DEBUG_TYPE "oft-manual-annotation-selection-analysis"

namespace oft {

void ManualAnnotationSelection::visitCallInst(llvm::CallInst &CInst) {
    LLVM_DEBUG(llvm::dbgs()
                   << "processing call instruction: " << CInst << "\n";);
    auto *func = CInst.getCalledFunction();

    // if it is a Fortran function call, this should return the Function
    if (!func) {
        func = llvm::cast<llvm::Function>(
            CInst.getCalledValue()->stripPointerCasts());
    }

    if (!func) {
        return;
    }

    if (func->getName() == ManualAnnotationFnName) {
        assert(CInst.getNumArgOperands() == ManualAnnotationFnArgsNum &&
               "mismatched number of arguments in manual annotation function");

        visitDefaultAnnotationFunc(CInst);
    } else {
        visitCustomFunc(CInst, func->getName());
    }
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

void ManualAnnotationSelection::visitCustomFunc(llvm::CallInst &CInst,
                                                llvm::StringRef FuncName) {
    auto found = std::find_if(
        std::begin(AnnotationDB), std::end(AnnotationDB), [&](const auto &e) {
            if (e.fnName == FuncName) {
                auto maxArg =
                    std::max_element(std::begin(e.fnArgs), std::end(e.fnArgs));

                return *maxArg <= CInst.getNumArgOperands();
            }

            return false;
        });

    if (found == AnnotationDB.end()) {
        return;
    }

    for (auto i : found->fnArgs) {
        Annotated.push_back(CInst.getArgOperand(i));
    }
}

ManualAnnotationSelection::Result ManualAnnotationSelection::getAnnotated() {
    ManualAnnotationSelection::Result res;
    res.values.insert(std::begin(Annotated), std::end(Annotated));

    return res;
}

} // namespace oft
