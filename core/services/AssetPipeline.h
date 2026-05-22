#pragma once

#include "core/foundation/Core.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Nyxty {

    enum class AssetPipelineAction {
        Copy,
        Validate,
        Compile
    };

    struct AssetPipelineRule {
        std::string name;
        std::string extension;
        AssetPipelineAction action{ AssetPipelineAction::Copy };
        std::filesystem::path outputSubdirectory;
    };

    struct AssetPipelineJob {
        std::filesystem::path sourcePath;
        std::filesystem::path outputPath;
        AssetPipelineAction action{ AssetPipelineAction::Copy };
        std::string ruleName;
        bool valid{ true };
        std::string message;
    };

    class NYXTY_CORE_API AssetPipeline {
    public:
        void SetRoots(std::filesystem::path sourceRoot, std::filesystem::path outputRoot);
        void AddRule(AssetPipelineRule rule);
        void ClearRules();

        std::vector<AssetPipelineJob> Plan() const;
        std::vector<AssetPipelineJob> PlanFile(const std::filesystem::path& sourcePath) const;

        const std::filesystem::path& GetSourceRoot() const { return m_SourceRoot; }
        const std::filesystem::path& GetOutputRoot() const { return m_OutputRoot; }
        const std::vector<AssetPipelineRule>& GetRules() const { return m_Rules; }

    private:
        const AssetPipelineRule* FindRule(const std::filesystem::path& sourcePath) const;

        std::filesystem::path m_SourceRoot{ "assets" };
        std::filesystem::path m_OutputRoot{ "out/assets" };
        std::vector<AssetPipelineRule> m_Rules;
    };

} // namespace Nyxty
