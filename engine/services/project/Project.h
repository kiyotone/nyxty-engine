#pragma once

#include "core/foundation/Core.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Nyxty {

    struct ProjectSettings {
        std::string name{ "Untitled Nyxty Project" };
        std::filesystem::path path;
        std::filesystem::path rootDirectory;
        std::filesystem::path assetDirectory{ "assets" };
        std::filesystem::path processedAssetDirectory{ "out/assets" };
        std::string buildProfile{ "debug" };
        std::vector<std::string> modules;
    };

    class NYXTY_CORE_API ProjectService {
    public:
        bool Load(const std::filesystem::path& path);
        bool Save(const std::filesystem::path& path = {}) const;

        void SetActive(ProjectSettings settings);
        const ProjectSettings& GetActive() const { return m_ActiveProject; }
        bool HasActiveProject() const { return !m_ActiveProject.path.empty(); }

    private:
        ProjectSettings m_ActiveProject;
    };

} // namespace Nyxty
