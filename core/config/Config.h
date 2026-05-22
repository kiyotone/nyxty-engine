#pragma once
#include "core/foundation/Base.h"
#include "core/foundation/Core.h"
#include <string>
#include <optional>
#include "json.hpp"

namespace Nyxty {

    class NYXTY_CORE_API Config {
    public:
        static Config& Instance();

        bool Load(const std::string& path);
        bool Save(const std::string& path = "");
        bool Has(const std::string& key) const;

        template<typename T>
        std::optional<T> Get(const std::string& key) const {
            const nlohmann::json* node = Resolve(key);
            if (!node) return std::nullopt;
            try { return node->get<T>(); }
            catch (...) { return std::nullopt; }
        }

        template<typename T>
        void Set(const std::string& key, const T& value) {
            nlohmann::json* node = ResolveOrCreate(key);
            if (!node) return;
            *node = value;
            NotifyChanged(key);
        }

    private:
        Config() = default;

        const nlohmann::json* Resolve(const std::string& key) const;
        nlohmann::json* ResolveOrCreate(const std::string& key);
        void                  NotifyChanged(const std::string& key);

        nlohmann::json m_Data;
        std::string    m_LoadedPath;
    };

} // namespace Nyxty