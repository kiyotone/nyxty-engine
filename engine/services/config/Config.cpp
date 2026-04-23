#include "Config.h"
#include "core/logging/Log.h"
#include "core/events/EventBus.h"
#include "core/events/EventBusInstance.h"
#include "core/events/Event.h"
#include "core/events/EventTypes.h"
#include <fstream>
#include <sstream>
#include <cstring>

namespace Nyxty {

    Config& Config::Instance() {
        static Config instance;
        return instance;
    }

    bool Config::Load(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            NYXTY_ENGINE_WARN("Config: Could not open '{}'", path);
            return false;
        }
        try {
            file >> m_Data;
            m_LoadedPath = path;
            NYXTY_ENGINE_INFO("Config: Loaded '{}'", path);
            return true;
        }
        catch (const std::exception& e) {
            NYXTY_ENGINE_ERROR("Config: Parse error in '{}': {}", path, e.what());
            return false;
        }
    }

    bool Config::Save(const std::string& path) {
        const std::string& target = path.empty() ? m_LoadedPath : path;
        if (target.empty()) {
            NYXTY_ENGINE_ERROR("Config: No save path specified");
            return false;
        }
        std::ofstream file(target);
        if (!file.is_open()) {
            NYXTY_ENGINE_ERROR("Config: Could not write to '{}'", target);
            return false;
        }
        file << m_Data.dump(4);
        NYXTY_ENGINE_INFO("Config: Saved to '{}'", target);
        return true;
    }

    bool Config::Has(const std::string& key) const {
        return Resolve(key) != nullptr;
    }

    const nlohmann::json* Config::Resolve(const std::string& key) const {
        const nlohmann::json* node = &m_Data;
        std::stringstream ss(key);
        std::string segment;
        while (std::getline(ss, segment, '.')) {
            if (!node->is_object() || !node->contains(segment))
                return nullptr;
            node = &(*node)[segment];
        }
        return node;
    }

    nlohmann::json* Config::ResolveOrCreate(const std::string& key) {
        nlohmann::json* node = &m_Data;
        std::stringstream ss(key);
        std::string segment;
        while (std::getline(ss, segment, '.')) {
            if (!node->contains(segment))
                (*node)[segment] = nlohmann::json::object();
            node = &(*node)[segment];
        }
        return node;
    }

    void Config::NotifyChanged(const std::string& key) {
        ConfigChangedEventData payload{};
        // Safe fixed-size copy, no overflow
        std::strncpy(payload.key, key.c_str(), sizeof(payload.key) - 1);

        auto evt = Event::Create<ConfigChangedEventData>(EVENT_CONFIG_CHANGED, payload);
		Nyxty::EventBusInstance::Get()->Dispatch(evt);
    }

} // namespace Nyxty