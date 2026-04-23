#include "services/assets/AssetPipeline.h"

#include "core/logging/Log.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace Nyxty {

namespace {
    std::string LowerExtension(std::filesystem::path path) {
        std::string extension = path.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return extension;
    }
}

void AssetPipeline::SetRoots(std::filesystem::path sourceRoot, std::filesystem::path outputRoot) {
    m_SourceRoot = std::move(sourceRoot);
    m_OutputRoot = std::move(outputRoot);
}

void AssetPipeline::AddRule(AssetPipelineRule rule) {
    std::transform(rule.extension.begin(), rule.extension.end(), rule.extension.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    m_Rules.push_back(std::move(rule));
}

void AssetPipeline::ClearRules() {
    m_Rules.clear();
}

std::vector<AssetPipelineJob> AssetPipeline::Plan() const {
    std::vector<AssetPipelineJob> jobs;
    if (!std::filesystem::exists(m_SourceRoot)) {
        NYXTY_ENGINE_WARN("AssetPipeline: source root '{}' does not exist.", m_SourceRoot.string());
        return jobs;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_SourceRoot)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        std::vector<AssetPipelineJob> fileJobs = PlanFile(entry.path());
        jobs.insert(jobs.end(), fileJobs.begin(), fileJobs.end());
    }
    return jobs;
}

std::vector<AssetPipelineJob> AssetPipeline::PlanFile(const std::filesystem::path& sourcePath) const {
    const AssetPipelineRule* rule = FindRule(sourcePath);
    if (!rule) {
        return {};
    }

    std::filesystem::path relative = sourcePath;
    std::error_code ec;
    if (!m_SourceRoot.empty()) {
        relative = std::filesystem::relative(sourcePath, m_SourceRoot, ec);
        if (ec) {
            relative = sourcePath.filename();
        }
    }

    AssetPipelineJob job{};
    job.sourcePath = sourcePath;
    job.action = rule->action;
    job.ruleName = rule->name;
    job.outputPath = m_OutputRoot / rule->outputSubdirectory / relative.filename();
    job.valid = true;
    return { job };
}

const AssetPipelineRule* AssetPipeline::FindRule(const std::filesystem::path& sourcePath) const {
    const std::string extension = LowerExtension(sourcePath);
    for (const AssetPipelineRule& rule : m_Rules) {
        if (rule.extension == extension) {
            return &rule;
        }
    }
    return nullptr;
}

} // namespace Nyxty
