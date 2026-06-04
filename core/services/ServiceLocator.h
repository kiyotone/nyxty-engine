#pragma once

#include "core/foundation/Core.h"
#include <memory>
#include <mutex>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Nyxty {

    class NYXTY_CORE_API ServiceLocator {
    public:
        template<typename T>
        static void Register(const std::string& key, T* service) {
            if (!service) { Unregister(key); return; }
            RegisterShared<T>(key, std::shared_ptr<T>(service, [](T*) {}));
        }

        template<typename T>
        static void RegisterShared(const std::string& key, std::shared_ptr<T> service) {
            std::scoped_lock lock(s_Mutex);
            s_Services[key] = Entry{ std::type_index(typeid(T)), std::move(service) };
        }

        template<typename T>
        static T* Get(const std::string& key) {
            return GetShared<T>(key).get();
        }

        template<typename T>
        static std::shared_ptr<T> GetShared(const std::string& key) {
            std::scoped_lock lock(s_Mutex);
            const auto it = s_Services.find(key);
            if (it == s_Services.end()) return {};
            if (it->second.type != std::type_index(typeid(T))) return {};
            return std::static_pointer_cast<T>(it->second.instance);
        }

        // Returns every service registered under type T exactly.
        template<typename T>
        static std::vector<T*> GetAll() {
            std::scoped_lock lock(s_Mutex);
            std::vector<T*> result;
            const auto target = std::type_index(typeid(T));
            for (auto& [key, entry] : s_Services) {
                if (entry.type == target)
                    result.push_back(static_cast<T*>(entry.instance.get()));
            }
            return result;
        }

        static bool Has(const std::string& key);
        static void Unregister(const std::string& key);
        static void Clear();

    private:
        struct Entry {
            std::type_index       type{ typeid(void) };
            std::shared_ptr<void> instance;
        };

        static std::unordered_map<std::string, Entry> s_Services;
        static std::mutex                              s_Mutex;
    };

} // namespace Nyxty
