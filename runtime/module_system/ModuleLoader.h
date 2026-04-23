#pragma once
#include "IModule.h"
#include "core/logging/Log.h"
#include "core/foundation/Base.h"
#include "core/foundation/Core.h"
#include "services/filesystem/FileWatcher.h"
#include "core/platform/Platform.h"

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#ifdef NYXTY_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace Nyxty {

    struct NYXTY_MODULE_API ModuleSnapshot {
        std::string name;
        u32 moduleID{ 0 };
        u32 versionMajor{ 0 };
        u32 versionMinor{ 0 };
        u32 versionPatch{ 0 };
        std::string path;
        std::string livePath;
        bool loaded{ false };
    };

    class NYXTY_MODULE_API ModuleLoader {
    public:
        ModuleLoader() = default;
        ~ModuleLoader() { StopWatcher(); Unload(); }

        std::vector<IModule*>    GetModules();
        std::vector<IModule*>    GetModules() const;
        std::vector<ModuleSnapshot> GetModuleSnapshots() const;
        bool                     Load(const std::string& path);
        bool                     Reload(const std::string& path);
        void                     Unload();
        void                     UnloadOne(const std::string& path);
        std::vector<std::string> DiscoverModules(
            const std::string& directory,
            const std::vector<std::string>& enabledIDs = {},
            const std::vector<std::string>& disabledIDs = {});

        // Hot-reload: call once after loading all modules.
        void StartWatcher(const std::string& directory);
        void StopWatcher();

        // Call this from the main loop to process pending reloads on the main thread.
        void ProcessPendingReloads();

    private:
        struct LoadedModule {
            std::string path;       // original path
            std::string livePath;   // temp copy path (avoids file lock on Windows)
#ifdef NYXTY_PLATFORM_WINDOWS
            HMODULE handle;
#else
            void* handle;
#endif
            IModule* instance;
        };

        bool         LoadInternal(const std::string& path, LoadedModule& out);
        void         UnloadModule(LoadedModule& mod);
        std::string  MakeLivePath(const std::string& path);

        std::vector<LoadedModule>    m_Modules;
        std::unique_ptr<FileWatcher> m_Watcher;

        // Pending reloads queued from watcher thread, processed on main thread
        std::vector<std::string>     m_PendingReloads;
        std::mutex                   m_PendingMutex;
    };

} // namespace Nyxty
