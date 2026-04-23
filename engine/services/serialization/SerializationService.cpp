#include "services/serialization/SerializationService.h"

#include "core/logging/Log.h"

#include <fstream>

namespace Nyxty {

bool SerializationService::SaveJson(const std::filesystem::path& path, const nlohmann::json& data) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        NYXTY_ENGINE_ERROR("SerializationService: could not write '{}'.", path.string());
        return false;
    }
    file << data.dump(4);
    return true;
}

std::optional<nlohmann::json> SerializationService::LoadJson(const std::filesystem::path& path) const {
    std::ifstream file(path);
    if (!file.is_open()) {
        NYXTY_ENGINE_WARN("SerializationService: could not open '{}'.", path.string());
        return std::nullopt;
    }

    nlohmann::json data;
    try {
        file >> data;
    } catch (const std::exception& ex) {
        NYXTY_ENGINE_ERROR("SerializationService: parse failed for '{}': {}", path.string(), ex.what());
        return std::nullopt;
    }

    return data;
}

bool SerializationService::ValidateJsonFile(const std::filesystem::path& path) const {
    return LoadJson(path).has_value();
}

} // namespace Nyxty
