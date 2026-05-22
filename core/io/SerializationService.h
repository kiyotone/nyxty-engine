#pragma once

#include "core/foundation/Core.h"
#include "json.hpp"

#include <filesystem>
#include <optional>

namespace Nyxty {

    class NYXTY_CORE_API SerializationService {
    public:
        bool SaveJson(const std::filesystem::path& path, const nlohmann::json& data) const;
        std::optional<nlohmann::json> LoadJson(const std::filesystem::path& path) const;
        bool ValidateJsonFile(const std::filesystem::path& path) const;
    };

} // namespace Nyxty
