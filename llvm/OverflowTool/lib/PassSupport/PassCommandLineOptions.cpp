//
//
//

#include "llvm/Support/CommandLine.h"
// using llvm::cl::opt
// using llvm::cl::list
// using llvm::cl::desc
// using llvm::cl::location
// using llvm::cl::cat
// using llvm::cl::OptionCategory

#include <string>
// using std::string

llvm::cl::OptionCategory
    OFTManualAnnotationCLCategory("Overflow Tool Pass",
                                  "Options for Overflow Tool pass");
