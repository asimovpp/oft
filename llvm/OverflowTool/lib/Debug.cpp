

#include "OverflowTool/Debug.hpp"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"

#include <list>
#include <string>

static bool OFTDebugFlag = false;

static llvm::cl::opt<std::string>
    OFTDebug("oft-debug", llvm::cl::init("false"), llvm::cl::ValueOptional,
             llvm::cl::callback([](const std::string &s) {
                 OFTDebugFlag = false;

                 auto sr = llvm::StringRef(s).trim().lower();
                 if (sr == "true" || sr.empty()) {
                     OFTDebugFlag = true;
                 }
             }),
             llvm::cl::desc("Enable debug output"));

static std::list<std::string> OFTDebugOnlyList;

static llvm::cl::list<std::string>
    OFTDebugOnly("oft-debug-only", llvm::cl::ValueRequired,
                 llvm::cl::CommaSeparated,
                 llvm::cl::callback([](const std::string &s) {
                     OFTDebugFlag = true;
                     OFTDebugOnlyList.push_front(s);
                 }),
                 llvm::cl::desc("Enable a specific type of debug output (comma "
                                "separated list of types)"));

#ifdef OFT_NDEBUG

bool oft_isDebug() { return false; }

bool oft_isCurrentDebugType(const char *) { return false; }

#else

bool oft_isDebug() { return OFTDebugFlag; }

bool oft_isCurrentDebugType(const char *DebugType) {
    if (OFTDebugOnlyList.empty()) {
        return true;
    }

    for (const auto &e : OFTDebugOnlyList) {
        if (e == DebugType) {
            return true;
        }
    }

    return false;
}

#endif // OFT_NDEBUG
