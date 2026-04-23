#include "services/project/Project.h"

#include "core/logging/Log.h"
#include "json.hpp"

#include <fstream>
#include <utility>

namespace Nyxty {

bool ProjectService::Load(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        NYXTY_ENGINE_WARN("ProjectService: could not open '{}'.", path.string());
        return false;
    }

    nlohmann::json data;
    try {
        file >> data;
    } catch (const std::exception& ex) {
        NYXTY_ENGINE_ERROR("ProjectService: parse failed for '{}': {}", path.string(), ex.what());
        return false;
    }

    ProjectSettings settings;
    settings.path = path;
    settings.name = data.value("name", settings.name);
    settings.rootDirectory = data.value("rootDirectory", path.parent_path().string());
    settings.assetDirectory = data.value("assetDirectory", settings.assetDirectory.string());
    settings.processedAssetDirectory = data.value("processedAssetDirectory", settings.processedAssetDirectory.string());
    settings.buildProfile = data.value("buildProfile", settings.buildProfile);

    if (data.contains("modules") && data["modules"].is_array()) {
        for (const auto& module : data["modules"]) {
            if (module.is_string()) {
                settings.modules.push_back(module.get<std::string>());
            }
        }
    }

    m_ActiveProject = std::move(settings);
    return true;
}

bool ProjectService::Save(const std::filesystem::path& path) const {
    const std::filesystem::path target = path.empty() ? m_ActiveProject.path : path;
    if (target.empty()) {
        return false;
    }

    nlohmann::json data;
    data["name"] = m_ActiveProject.name;
    data["rootDirectory"] = m_ActiveProject.rootDirectory.string();
    data["assetDirectory"] = m_ActiveProject.assetDirectory.string();
    data["processedAssetDirectory"] = m_ActiveProject.processedAssetDirectory.string();
    data["buildProfile"] = m_ActiveProject.buildProfile;
    data["modules"] = m_ActiveProject.modules;

    std::ofstream file(target);
    if (!file.is_open()) {
        NYXTY_ENGINE_ERROR("ProjectService: could not write '{}'.", target.string());
        return false;
    }

    file << data.dump(4);
    return true;
}

void ProjectService::SetActive(ProjectSettings settings) {
    m_ActiveProject = std::move(settings);
}

} // namespace Nyxty
