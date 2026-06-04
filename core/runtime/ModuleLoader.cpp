#include "ModuleLoader.h"

#include "core/events/Event.h"
#include "core/events/EventBusInstance.h"
#include "core/events/EventTypes.h"
#include "core/foundation/Base.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <queue>
#include <set>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#ifndef NYXTY_PLATFORM_WINDOWS
#include <dlfcn.h>
#endif

#include "json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace Nyxty {

    namespace {

        struct ManifestEntry {
            std::string id;
            std::string name;
            std::string description;
            std::string version;
            std::string library;
            std::vector<std::string> dependencies;
            int loadPriority{ 0 };
            bool enabledByDefault{ true };
            fs::path manifestPath;
            fs::path libraryPath;
        };

        std::atomic_uint64_t s_LiveCopyCounter{ 0 };

        bool IsModuleBinary(const fs::path& path) {
#ifdef NYXTY_PLATFORM_WINDOWS
            return path.extension() == ".dll" && path.stem().string().find("_live") == std::string::npos;
#else
            return path.extension() == ".so" && path.stem().string().find("_live") == std::string::npos;
#endif
        }

        std::string StripModuleSuffix(const std::string& value) {
            constexpr std::string_view kManifestSuffix = ".module.json";
            if (value.size() >= kManifestSuffix.size() &&
                value.compare(value.size() - kManifestSuffix.size(), kManifestSuffix.size(), kManifestSuffix) == 0) {
                return value.substr(0, value.size() - kManifestSuffix.size());
            }
            return value;
        }

        std::string NormalizeModuleKey(const fs::path& path) {
            std::string stem = StripModuleSuffix(path.filename().string());
            fs::path stemPath(stem);
            stem = stemPath.stem().string().empty() ? stem : stemPath.stem().string();

            const auto livePos = stem.find("_live");
            if (livePos != std::string::npos) {
                stem = stem.substr(0, livePos);
            }

#ifdef NYXTY_PLATFORM_WINDOWS
            if (!stem.empty() && stem.back() == 'd') {
                stem.pop_back();
            }
#endif
            return stem;
        }

        fs::path ResolveModuleBinaryPath(const fs::path& manifestPath, const std::string& library) {
            const fs::path manifestDir = manifestPath.parent_path();
            const fs::path rawLibrary(library);

            const auto tryBinary = [](const fs::path& candidate) -> fs::path {
                std::error_code ec;
                if (!candidate.empty() && fs::exists(candidate, ec) && fs::is_regular_file(candidate, ec) && IsModuleBinary(candidate)) {
                    return candidate;
                }
                return {};
            };

            if (rawLibrary.is_absolute()) {
                if (fs::path exact = tryBinary(rawLibrary); !exact.empty()) {
                    return exact;
                }
            }

            if (rawLibrary.has_extension()) {
                if (fs::path relative = tryBinary(manifestDir / rawLibrary); !relative.empty()) {
                    return relative;
                }
                if (fs::path exact = tryBinary(rawLibrary); !exact.empty()) {
                    return exact;
                }
            }

            const fs::path searchDirectory = rawLibrary.has_parent_path() ? (manifestDir / rawLibrary.parent_path()) : manifestDir;
            const std::string moduleKey = NormalizeModuleKey(rawLibrary.filename());
            std::vector<fs::directory_entry> candidates;

            std::error_code ec;
            if (!fs::exists(searchDirectory, ec) || !fs::is_directory(searchDirectory, ec)) {
                return {};
            }

            for (const auto& entry : fs::directory_iterator(searchDirectory, ec)) {
                if (ec || !entry.is_regular_file(ec)) {
                    continue;
                }

                if (!IsModuleBinary(entry.path())) {
                    continue;
                }

                if (NormalizeModuleKey(entry.path()) == moduleKey) {
                    candidates.push_back(entry);
                }
            }

            if (candidates.empty()) {
                return {};
            }

            std::sort(candidates.begin(), candidates.end(), [](const fs::directory_entry& lhs, const fs::directory_entry& rhs) {
                std::error_code lhsEc;
                std::error_code rhsEc;
                const auto lhsTime = lhs.last_write_time(lhsEc);
                const auto rhsTime = rhs.last_write_time(rhsEc);

                if (lhsEc || rhsEc) {
                    return lhs.path().string() < rhs.path().string();
                }

                if (lhsTime == rhsTime) {
                    return lhs.path().string() < rhs.path().string();
                }

                return lhsTime > rhsTime;
            });

            return candidates.front().path();
        }

        std::optional<ManifestEntry> ParseManifest(const fs::path& manifestPath) {
            std::ifstream stream(manifestPath);
            if (!stream.is_open()) {
                NYXTY_ENGINE_WARN("ModuleLoader: Failed to open manifest '{}'.", manifestPath.string());
                return std::nullopt;
            }

            json data;
            try {
                stream >> data;
            } catch (const std::exception& ex) {
                NYXTY_ENGINE_ERROR("ModuleLoader: Failed to parse manifest '{}': {}", manifestPath.string(), ex.what());
                return std::nullopt;
            }

            ManifestEntry entry{};
            entry.manifestPath = manifestPath;
            entry.id = data.value("id", NormalizeModuleKey(manifestPath));
            entry.name = data.value("name", entry.id);
            entry.description = data.value("description", std::string{});
            entry.version = data.value("version", std::string("0.0.0"));
            entry.library = data.value("library", entry.name);
            entry.loadPriority = data.value("load_priority", 0);
            entry.enabledByDefault = data.value("enabled", true);

            if (data.contains("dependencies") && data["dependencies"].is_array()) {
                for (const auto& dependency : data["dependencies"]) {
                    if (dependency.is_string()) {
                        entry.dependencies.push_back(dependency.get<std::string>());
                    }
                }
            }

            entry.libraryPath = ResolveModuleBinaryPath(manifestPath, entry.library);
            if (entry.libraryPath.empty()) {
                NYXTY_ENGINE_WARN(
                    "ModuleLoader: Manifest '{}' did not resolve a module binary for '{}'.",
                    manifestPath.string(),
                    entry.id);
                return std::nullopt;
            }

            return entry;
        }

        std::vector<ManifestEntry> DiscoverManifestEntries(const std::string& directory) {
            std::vector<ManifestEntry> manifests;
            std::error_code ec;

            for (fs::recursive_directory_iterator it(directory, ec), end; !ec && it != end; it.increment(ec)) {
                const fs::directory_entry& entry = *it;
                if (!entry.is_regular_file(ec)) {
                    continue;
                }

                const std::string fileName = entry.path().filename().string();
                if (fileName.size() < std::string(".module.json").size() ||
                    fileName.rfind(".module.json") != fileName.size() - std::string(".module.json").size()) {
                    continue;
                }

                if (auto manifest = ParseManifest(entry.path())) {
                    manifests.push_back(std::move(*manifest));
                }
            }

            return manifests;
        }

        std::vector<std::string> DiscoverLegacyModuleBinaries(const std::string& directory) {
            std::vector<std::string> result;
            if (!fs::exists(directory)) {
                NYXTY_ENGINE_WARN("ModuleLoader: Module directory not found: {}", directory);
                return result;
            }

            std::unordered_map<std::string, fs::directory_entry> selectedEntries;
            for (const auto& entry : fs::directory_iterator(directory)) {
                const auto& path = entry.path();
                if (!IsModuleBinary(path)) {
                    continue;
                }

                const std::string moduleKey = NormalizeModuleKey(path);
                auto it = selectedEntries.find(moduleKey);
                if (it == selectedEntries.end()) {
                    selectedEntries.emplace(moduleKey, entry);
                    continue;
                }

                std::error_code ec;
                const auto currentWriteTime = entry.last_write_time(ec);
                if (ec) {
                    continue;
                }

                ec.clear();
                const auto selectedWriteTime = it->second.last_write_time(ec);
                if (ec || currentWriteTime > selectedWriteTime) {
                    NYXTY_ENGINE_WARN(
                        "ModuleLoader: Multiple variants found for legacy module '{}'; selecting '{}'.",
                        moduleKey,
                        path.string());
                    it->second = entry;
                }
            }

            result.reserve(selectedEntries.size());
            for (const auto& [_, entry] : selectedEntries) {
                result.push_back(entry.path().string());
            }

            std::sort(result.begin(), result.end());
            return result;
        }

        std::vector<std::string> ResolveManifestLoadOrder(
            const std::vector<ManifestEntry>& discovered,
            const std::vector<std::string>& enabledIDs,
            const std::vector<std::string>& disabledIDs) {

            std::unordered_set<std::string> enabledSet(enabledIDs.begin(), enabledIDs.end());
            std::unordered_set<std::string> disabledSet(disabledIDs.begin(), disabledIDs.end());
            std::unordered_map<std::string, ManifestEntry> selected;

            for (const auto& manifest : discovered) {
                if (disabledSet.contains(manifest.id)) {
                    continue;
                }

                const bool enabled = enabledSet.empty() ? manifest.enabledByDefault : enabledSet.contains(manifest.id);
                if (!enabled) {
                    continue;
                }

                const auto [it, inserted] = selected.emplace(manifest.id, manifest);
                if (!inserted) {
                    NYXTY_ENGINE_WARN(
                        "ModuleLoader: Duplicate manifest id '{}' found in '{}' and '{}'. Keeping the first entry.",
                        manifest.id,
                        it->second.manifestPath.string(),
                        manifest.manifestPath.string());
                }
            }

            bool removedAny = false;
            do {
                removedAny = false;
                std::vector<std::string> toRemove;

                for (const auto& [id, manifest] : selected) {
                    const auto dependencyIt = std::find_if(
                        manifest.dependencies.begin(),
                        manifest.dependencies.end(),
                        [&selected](const std::string& dependency) { return !selected.contains(dependency); });

                    if (dependencyIt != manifest.dependencies.end()) {
                        NYXTY_ENGINE_WARN(
                            "ModuleLoader: Skipping module '{}' because dependency '{}' is not enabled or missing.",
                            id,
                            *dependencyIt);
                        toRemove.push_back(id);
                    }
                }

                for (const auto& id : toRemove) {
                    removedAny = true;
                    selected.erase(id);
                }
            } while (removedAny);

            std::unordered_map<std::string, int> indegree;
            std::unordered_map<std::string, std::vector<std::string>> dependents;
            for (const auto& [id, manifest] : selected) {
                indegree[id] = 0;
            }

            for (const auto& [id, manifest] : selected) {
                for (const auto& dependency : manifest.dependencies) {
                    ++indegree[id];
                    dependents[dependency].push_back(id);
                }
            }

            auto manifestComparator = [&selected](const std::string& lhs, const std::string& rhs) {
                const auto& left = selected.at(lhs);
                const auto& right = selected.at(rhs);
                if (left.loadPriority != right.loadPriority) {
                    return left.loadPriority < right.loadPriority;
                }
                return left.id < right.id;
            };

            std::vector<std::string> ready;
            ready.reserve(indegree.size());
            for (const auto& [id, degree] : indegree) {
                if (degree == 0) {
                    ready.push_back(id);
                }
            }
            std::sort(ready.begin(), ready.end(), manifestComparator);

            std::vector<std::string> orderedIDs;
            orderedIDs.reserve(selected.size());

            while (!ready.empty()) {
                const std::string id = ready.front();
                ready.erase(ready.begin());
                orderedIDs.push_back(id);

                auto dependentIt = dependents.find(id);
                if (dependentIt == dependents.end()) {
                    continue;
                }

                for (const auto& dependent : dependentIt->second) {
                    auto indegreeIt = indegree.find(dependent);
                    if (indegreeIt == indegree.end()) {
                        continue;
                    }

                    --indegreeIt->second;
                    if (indegreeIt->second == 0) {
                        ready.push_back(dependent);
                    }
                }

                std::sort(ready.begin(), ready.end(), manifestComparator);
            }

            if (orderedIDs.size() != selected.size()) {
                NYXTY_ENGINE_WARN("ModuleLoader: Manifest dependency cycle detected. Falling back to priority order.");

                orderedIDs.clear();
                for (const auto& [id, _] : selected) {
                    orderedIDs.push_back(id);
                }
                std::sort(orderedIDs.begin(), orderedIDs.end(), manifestComparator);
            }

            std::vector<std::string> result;
            result.reserve(orderedIDs.size());
            for (const auto& id : orderedIDs) {
                result.push_back(selected.at(id).libraryPath.string());
            }
            return result;
        }

    } // namespace

    std::string ModuleLoader::MakeLivePath(const std::string& path) {
        fs::path p(path);
        const auto suffix = s_LiveCopyCounter.fetch_add(1, std::memory_order_relaxed);
        return (p.parent_path() / (p.stem().string() + "_live_" + std::to_string(suffix) + p.extension().string())).string();
    }

    bool ModuleLoader::LoadInternal(const std::string& path, LoadedModule& out) {
        std::string livePath = MakeLivePath(path);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::error_code ec;
        fs::copy_file(path, livePath, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            NYXTY_ENGINE_ERROR("ModuleLoader: Failed to copy {} -> {}: {}", path, livePath, ec.message());
            return false;
        }

#ifdef NYXTY_PLATFORM_WINDOWS
        HMODULE handle = ::LoadLibraryA(livePath.c_str());
        if (!handle) {
            NYXTY_ENGINE_ERROR("ModuleLoader: LoadLibrary failed for {}", livePath);
            return false;
        }
        auto createFn = (IModule * (*)())  ::GetProcAddress(handle, "CreateModule");
        auto destroyFn = (void(*)(IModule*))::GetProcAddress(handle, "DestroyModule");
#else
        void* handle = ::dlopen(livePath.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!handle) {
            NYXTY_ENGINE_ERROR("ModuleLoader: dlopen failed: {}", ::dlerror());
            return false;
        }
        auto createFn = (IModule * (*)())  ::dlsym(handle, "CreateModule");
        auto destroyFn = (void(*)(IModule*))::dlsym(handle, "DestroyModule");
#endif

        if (!createFn || !destroyFn) {
            NYXTY_ENGINE_ERROR("ModuleLoader: Missing CreateModule/DestroyModule in {}", path);
            LoadedModule temp{};
            temp.path = path;
            temp.livePath = livePath;
            temp.handle = handle;
            temp.instance = nullptr;
            UnloadModule(temp);
            return false;
        }

        out.path = path;
        out.livePath = livePath;
        out.handle = handle;
        out.instance = createFn();
        if (!out.instance) {
            NYXTY_ENGINE_ERROR("ModuleLoader: CreateModule returned null for {}", path);
            UnloadModule(out);
            return false;
        }
        return true;
    }

    void ModuleLoader::UnloadModule(LoadedModule& mod) {
        if (mod.instance) {
#ifdef NYXTY_PLATFORM_WINDOWS
            auto destroyFn = (void(*)(IModule*))::GetProcAddress(mod.handle, "DestroyModule");
#else
            auto destroyFn = (void(*)(IModule*))::dlsym(mod.handle, "DestroyModule");
#endif
            if (destroyFn) {
                destroyFn(mod.instance);
            }
            mod.instance = nullptr;
        }

#ifdef NYXTY_PLATFORM_WINDOWS
        if (mod.handle) {
            ::FreeLibrary(mod.handle);
            mod.handle = nullptr;
        }
#else
        if (mod.handle) {
            ::dlclose(mod.handle);
            mod.handle = nullptr;
        }
#endif

        std::error_code ec;
        fs::remove(mod.livePath, ec);
    }

    std::vector<IModule*> ModuleLoader::GetModules() {
        std::vector<IModule*> result;
        for (auto& module : m_Modules) {
            if (module.instance) {
                result.push_back(module.instance);
            }
        }
        return result;
    }

    std::vector<IModule*> ModuleLoader::GetModules() const {
        std::vector<IModule*> result;
        for (const auto& module : m_Modules) {
            if (module.instance) {
                result.push_back(module.instance);
            }
        }
        return result;
    }

    std::vector<ModuleSnapshot> ModuleLoader::GetModuleSnapshots() const {
        std::vector<ModuleSnapshot> snapshots;
        snapshots.reserve(m_Modules.size());

        for (const auto& module : m_Modules) {
            ModuleSnapshot snapshot{};
            snapshot.path = module.path;
            snapshot.livePath = module.livePath;
            snapshot.loaded = module.instance != nullptr;

            if (module.instance) {
                snapshot.name = module.instance->GetName();
                snapshot.moduleID = module.instance->GetModuleID();
                snapshot.versionMajor = module.instance->GetVersionMajor();
                snapshot.versionMinor = module.instance->GetVersionMinor();
                snapshot.versionPatch = module.instance->GetVersionPatch();
            }

            snapshots.push_back(std::move(snapshot));
        }

        std::sort(snapshots.begin(), snapshots.end(), [](const ModuleSnapshot& lhs, const ModuleSnapshot& rhs) {
            if (lhs.moduleID != rhs.moduleID) {
                return lhs.moduleID < rhs.moduleID;
            }
            return lhs.name < rhs.name;
        });
        return snapshots;
    }

    bool ModuleLoader::Load(const std::string& path) {
        LoadedModule module{};
        if (!LoadInternal(path, module)) {
            return false;
        }

        if (!module.instance->OnLoad()) {
            NYXTY_ENGINE_ERROR("ModuleLoader: OnLoad failed for {}", path);
            UnloadModule(module);
            return false;
        }

        m_Modules.push_back(module);
        NYXTY_ENGINE_INFO("ModuleLoader: Loaded {}", path);

        ModuleLoadedEventData eventData{};
        eventData.moduleID = module.instance->GetModuleID();
        std::strncpy(eventData.moduleName, module.instance->GetName(), sizeof(eventData.moduleName) - 1);
        EventBusInstance::Get()->Dispatch(Event::Create(EVENT_MODULE_LOADED, eventData, eventData.moduleID));
        return true;
    }

    bool ModuleLoader::Reload(const std::string& path) {
        NYXTY_ENGINE_INFO("ModuleLoader: Reloading {}", path);

        auto it = std::find_if(m_Modules.begin(), m_Modules.end(),
            [&path](const LoadedModule& module) { return module.path == path; });

        if (it == m_Modules.end()) {
            NYXTY_ENGINE_WARN("ModuleLoader: Cannot reload - not loaded: {}", path);
            return false;
        }

        auto* eventBus = EventBusInstance::Get();
        eventBus->UnsubscribeAll(it->instance->GetModuleID());
        it->instance->OnUnload();
        UnloadModule(*it);

        LoadedModule newModule{};
        if (!LoadInternal(path, newModule)) {
            NYXTY_ENGINE_ERROR("ModuleLoader: Reload failed for {}", path);
            m_Modules.erase(it);
            return false;
        }

        *it = newModule;
        if (!it->instance->OnLoad()) {
            NYXTY_ENGINE_ERROR("ModuleLoader: OnLoad failed during reload for {}", path);
            UnloadModule(*it);
            m_Modules.erase(it);
            return false;
        }

        auto evt = Event::Create<u32>(EVENT_MODULE_RELOAD);
        eventBus->Dispatch(evt);

        NYXTY_ENGINE_INFO("ModuleLoader: Reload complete for {}", path);
        return true;
    }

    void ModuleLoader::UnloadOne(const std::string& path) {
        auto it = std::find_if(m_Modules.begin(), m_Modules.end(),
            [&path](const LoadedModule& module) { return module.path == path; });
        if (it == m_Modules.end()) {
            return;
        }

        EventBusInstance::Get()->UnsubscribeAll(it->instance->GetModuleID());
        it->instance->OnUnload();
        UnloadModule(*it);
        m_Modules.erase(it);
    }

    void ModuleLoader::Unload() {
        // Unload in REVERSE load order so dependencies are always live when a
        // module shuts down (e.g. SDL must outlive ImGui, bgfx must outlive shaders).
        for (auto it = m_Modules.rbegin(); it != m_Modules.rend(); ++it) {
            if (it->instance) {
                EventBusInstance::Get()->UnsubscribeAll(it->instance->GetModuleID());
                it->instance->OnUnload();
            }
            UnloadModule(*it);
        }
        m_Modules.clear();
    }

    std::vector<std::string> ModuleLoader::DiscoverModules(
        const std::string& directory,
        const std::vector<std::string>& enabledIDs,
        const std::vector<std::string>& disabledIDs) {

        if (!fs::exists(directory)) {
            NYXTY_ENGINE_WARN("ModuleLoader: Module directory not found: {}", directory);
            return {};
        }

        const auto manifests = DiscoverManifestEntries(directory);
        if (!manifests.empty()) {
            auto orderedModules = ResolveManifestLoadOrder(manifests, enabledIDs, disabledIDs);
            if (orderedModules.empty()) {
                NYXTY_ENGINE_WARN("ModuleLoader: No manifest-enabled modules were selected in '{}'.", directory);
            }
            return orderedModules;
        }

        NYXTY_ENGINE_WARN("ModuleLoader: No module manifests found in '{}'. Falling back to legacy DLL discovery.", directory);
        return DiscoverLegacyModuleBinaries(directory);
    }

    void ModuleLoader::StartWatcher(const std::string& directory) {
        m_Watcher = std::make_unique<FileWatcher>();
        m_Watcher->Start(directory, [this](const std::filesystem::path& path) {
            const std::string pathString = path.string();
            if (pathString.find("_live") != std::string::npos) {
                return;
            }

            if (!IsModuleBinary(path)) {
                return;
            }

            std::lock_guard lock(m_PendingMutex);
            if (std::find(m_PendingReloads.begin(), m_PendingReloads.end(), pathString) == m_PendingReloads.end()) {
                m_PendingReloads.push_back(pathString);
            }
        });
        NYXTY_ENGINE_INFO("ModuleLoader: Hot-reload watcher started on {}", directory);
    }

    void ModuleLoader::StopWatcher() {
        if (m_Watcher) {
            m_Watcher->Stop();
            m_Watcher.reset();
        }
    }

    void ModuleLoader::ProcessPendingReloads() {
        std::vector<std::string> pending;
        {
            std::lock_guard lock(m_PendingMutex);
            std::swap(pending, m_PendingReloads);
        }

        for (const auto& path : pending) {
            Reload(path);
        }
    }

} // namespace Nyxty
