#pragma once

#include "core/platform/Platform.h"

// ========================================
// Platform-Specific DLL Export/Import
// ========================================
#ifdef NYXTY_PLATFORM_WINDOWS
#ifdef NYXTY_CORE_BUILD
#define NYXTY_CORE_API __declspec(dllexport)
#else
#define NYXTY_CORE_API __declspec(dllimport)
#endif

#ifdef NYXTY_MODULE_BUILD
#define NYXTY_MODULE_API __declspec(dllexport)
#else
#define NYXTY_MODULE_API __declspec(dllimport)
#endif
#else
#define NYXTY_CORE_API
#define NYXTY_MODULE_API
#endif

// ========================================
// Debug Break
// ========================================
#ifdef NYXTY_PLATFORM_WINDOWS
#define NYXTY_DEBUGBREAK() __debugbreak()
#elif defined(NYXTY_PLATFORM_LINUX) || defined(NYXTY_PLATFORM_MACOS)
#include <signal.h>
#define NYXTY_DEBUGBREAK() raise(SIGTRAP)
#else
#define NYXTY_DEBUGBREAK()
#endif

// ========================================
// Enable Asserts
// ========================================
#ifdef NYXTY_DEBUG
#define NYXTY_ENABLE_ASSERTS
#endif

// ========================================
// Profiling (Optional - for future)
// ========================================
#ifdef NYXTY_ENABLE_PROFILING
#define NYXTY_PROFILE_SCOPE(name)    // TODO: Implement profiling
#define NYXTY_PROFILE_FUNCTION()     // TODO: Implement profiling
#else
#define NYXTY_PROFILE_SCOPE(name)
#define NYXTY_PROFILE_FUNCTION()
#endif