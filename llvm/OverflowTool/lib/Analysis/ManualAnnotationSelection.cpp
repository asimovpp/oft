#include "OverflowTool/Analysis/ManualAnnotationSelection.hpp"

#include "OverflowTool/Debug.hpp"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"

#include <algorithm>

#define DEBUG_TYPE "oft-manual-annotation-selection-analysis"

namespace oft {

void ManualAnnotationSelection::visitCallInst(llvm::CallInst &CInst) {
    OFT_DEBUG(llvm::dbgs() << "processing call instruction: " << CInst
                           << "\n";);
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
    auto *op0 = CInst.arg_begin()->get(); //->stripPointerCasts();
    Annotated.push_back(op0);
    return;
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

                // the function matches if:
                // a. its return value should be annotated, or
                // b. if specified, any of its arguments
                return e.retVal || ((maxArg != std::end(e.fnArgs)) &&
                                    (*maxArg <= CInst.getNumArgOperands()));
            }

            return false;
        });

    if (found == AnnotationDB.end()) {
        return;
    }

    if (found->retVal) {
        Annotated.push_back(&CInst);
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
