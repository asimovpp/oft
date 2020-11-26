#pragma once

#include <string>

#include "llvm/IR/Instruction.h"

namespace oft {
    std::string getFunctionName(llvm::Instruction* inst);
}
