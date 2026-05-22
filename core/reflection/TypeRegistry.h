#pragma once

#include "core/foundation/Core.h"
#include <string>
#include <typeindex>
#include <unordered_map>

namespace Nyxty {

    // Placeholder for future type registry / reflection system.
    // Will support: type name lookup, property enumeration, serialization, scripting bindings.
    class NYXTY_CORE_API TypeRegistry {
    public:
        static TypeRegistry& Instance() {
            static TypeRegistry instance;
            return instance;
        }

        template<typename T>
        void Register(const std::string& name) {
            m_Names[std::type_index(typeid(T))] = name;
        }

        template<typename T>
        const std::string* GetName() const {
            const auto it = m_Names.find(std::type_index(typeid(T)));
            return it != m_Names.end() ? &it->second : nullptr;
        }

    private:
        TypeRegistry() = default;
        std::unordered_map<std::type_index, std::string> m_Names;
    };

} // namespace Nyxty
