#pragma once

#include <cassert>

// ========================================
// Platform Detection
// ========================================
#ifdef _WIN32
#ifdef _WIN64
#define NYXTY_PLATFORM_WINDOWS
#else
#error "x86 builds are not supported!"
#endif
#elif defined(__APPLE__) || defined(__MACH__)
#include <TargetConditionals.h>
#if TARGET_OS_MAC == 1
#define NYXTY_PLATFORM_MACOS
#else
#error "Only macOS is supported!"
#endif
#elif defined(__linux__)
#define NYXTY_PLATFORM_LINUX
#else
#error "Unknown platform!"
#endif

// ========================================
// Debug Configuration
// ========================================
#if defined(_DEBUG) || defined(DEBUG)
#define NYXTY_DEBUG
#endif

#if defined(NDEBUG) || defined(_NDEBUG)
#define NYXTY_RELEASE
#endif

// ========================================
// DLL Export / Import Macro
// ========================================
#ifdef NYXTY_PLATFORM_WINDOWS
#ifdef NYXTY_PLATFORM_BUILD      // Defined when building the DLL
#define PLATFORM_EXPORT __declspec(dllexport)
#else
#define PLATFORM_EXPORT __declspec(dllimport)
#endif
#else
#define PLATFORM_EXPORT          // For Linux/macOS (no decoration needed)
#endif
