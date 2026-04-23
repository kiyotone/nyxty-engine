#pragma once

#include "core/foundation/Core.h"
#include <memory>
#include <cstdint>
#include <filesystem>
#include <utility>


// ========================================
// Smart Pointer Aliases
// ========================================
namespace Nyxty {

    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T, typename... Args>
    constexpr Scope<T> CreateScope(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T, typename... Args>
    constexpr Ref<T> CreateRef(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

} // namespace Nyxty

// ========================================
// Bit Manipulation
// ========================================
#define BIT(x) (1 << (x))

// ========================================
// Event Binding
// ========================================
#define NYXTY_BIND_EVENT_FN(fn) \
    [this](auto&&... args) -> decltype(auto) { \
        return this->fn(std::forward<decltype(args)>(args)...); \
    }

// ========================================
// Module Export
// ========================================
#ifdef NYXTY_PLATFORM_WINDOWS
#define MODULE_EXPORT extern "C" __declspec(dllexport)
#else
#define MODULE_EXPORT extern "C" __attribute__((visibility("default")))
#endif

// ========================================
// Common Type Aliases
// ========================================
namespace Nyxty {

    using byte = uint8_t;

    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

    using f32 = float;
    using f64 = double;

} // namespace Nyxty
