#pragma once

#include "core/foundation/Core.h"
#include "core/logging/Log.h"

// ========================================
// Assertion Macros
// ========================================
#ifdef NYXTY_ENABLE_ASSERTS
    // Core assertions (for engine code)
#define NYXTY_CORE_ASSERT(x, ...) { \
        if(!(x)) { \
            NYXTY_ENGINE_ERROR("Assertion Failed: {0}", __VA_ARGS__); \
            NYXTY_DEBUGBREAK(); \
        } \
    }

    // Client assertions (for application code)
#define NYXTY_ASSERT(x, ...) { \
        if(!(x)) { \
            NYXTY_APP_ERROR("Assertion Failed: {0}", __VA_ARGS__); \
            NYXTY_DEBUGBREAK(); \
        } \
    }
#else
#define NYXTY_CORE_ASSERT(x, ...)
#define NYXTY_ASSERT(x, ...)
#endif

// ========================================
// Verify Macro (Always enabled, even in release)
// ========================================
#define NYXTY_CORE_VERIFY(x, ...) { \
    if(!(x)) { \
        NYXTY_ENGINE_ERROR("Verification Failed: {0}", __VA_ARGS__); \
        NYXTY_DEBUGBREAK(); \
    } \
}

#define NYXTY_VERIFY(x, ...) { \
    if(!(x)) { \
        NYXTY_APP_ERROR("Verification Failed: {0}", __VA_ARGS__); \
        NYXTY_DEBUGBREAK(); \
    } \
}
