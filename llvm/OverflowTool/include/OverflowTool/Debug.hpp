#pragma once

#include "OverflowTool/Config.hpp"

#include "llvm/Support/Debug.h"

bool oft_isDebug();
bool oft_isCurrentDebugType(const char *DebugType);

#define OFT_DEBUG_WITH_TYPE(TYPE, X)                                           \
    do {                                                                       \
        if (oft_isDebug() && oft_isCurrentDebugType(TYPE)) {                   \
            X;                                                                 \
        }                                                                      \
    } while (false)

#define OFT_DEBUG(X) OFT_DEBUG_WITH_TYPE(DEBUG_TYPE, X)
