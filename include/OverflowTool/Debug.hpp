#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/Support/Debug.h"

bool oft_isDebug();
bool oft_isCurrentDebugType(const char *DebugType);

// OFT_NDEBUG has similar functionality as NDEBUG, but without currently
// affecting assertions. This allows decoupling of debug facilities from LLVM.

#ifdef OFT_NDEBUG

#define OFT_DEBUG_WITH_TYPE(TYPE, X)                                           \
    do {                                                                       \
    } while (false)

#else

#define OFT_DEBUG_WITH_TYPE(TYPE, X)                                           \
    do {                                                                       \
        if (oft_isDebug() && oft_isCurrentDebugType(TYPE)) {                   \
            X;                                                                 \
        }                                                                      \
    } while (false)

#endif // OFT_NDEBUG

#define OFT_DEBUG(X) OFT_DEBUG_WITH_TYPE(DEBUG_TYPE, X)
