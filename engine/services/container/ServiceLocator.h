#pragma once

#include "core/foundation/Core.h"
#include "rendering/backend/Renderer.h"
#include "services/window/Window.h"
#include <SDL3/SDL_render.h>
#include <memory>
#include <mutex>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace Nyxty {
    class AssetPipeline;
    class NotificationCenter;
    class ProjectService;
    class ScriptHost;
    class SerializationService;
    class TaskSystem;
    class TimeSystem;

    class NYXTY_CORE_API ServiceLocator {
    public:
        // Called once by main() after the main window and renderer are created.
        static void Provide(Window* mainWindow, Renderer* renderer, SDL_Renderer* uiRenderer);
        static void ProvideAssets(AssetPipeline* assetPipeline);
        static void ProvideNotifications(NotificationCenter* notificationCenter);
        static void ProvideProject(ProjectService* projectService);
        static void ProvideScripting(ScriptHost* scriptHost);
        static void ProvideSerialization(SerializationService* serializationService);
        static void ProvideTasks(TaskSystem* taskSystem);
        static void ProvideTime(TimeSystem* timeSystem);

        template<typename T>
        static void Register(const std::string& key, T* service) {
            if (!service) {
                Unregister(key);
                return;
            }

            RegisterShared<T>(key, std::shared_ptr<T>(service, [](T*) {}));
        }

        template<typename T>
        static void RegisterShared(const std::string& key, std::shared_ptr<T> service) {
            std::scoped_lock lock(s_ServiceMutex);
            s_Services[key] = ServiceEntry{ std::type_index(typeid(T)), std::move(service) };
        }

        template<typename T>
        static T* Get(const std::string& key) {
            return GetShared<T>(key).get();
        }

        template<typename T>
        static std::shared_ptr<T> GetShared(const std::string& key) {
            std::scoped_lock lock(s_ServiceMutex);

            const auto it = s_Services.find(key);
            if (it == s_Services.end()) {
                return {};
            }

            if (it->second.type != std::type_index(typeid(T))) {
                return {};
            }

            return std::static_pointer_cast<T>(it->second.instance);
        }

        static bool Has(const std::string& key);
        static void Unregister(const std::string& key);
        static void Clear();

        static Window* GetMainWindow();
        static AssetPipeline* GetAssetPipeline();
        static NotificationCenter* GetNotificationCenter();
        static ProjectService* GetProjectService();
        static Renderer* GetRenderer();
        static ScriptHost* GetScriptHost();
        static SerializationService* GetSerializationService();
        static SDL_Renderer* GetUiRenderer();
        static TaskSystem* GetTaskSystem();
        static TimeSystem* GetTimeSystem();

    private:
        struct ServiceEntry {
            std::type_index type{ typeid(void) };
            std::shared_ptr<void> instance;
        };

        static constexpr const char* kAssetPipelineService = "assets.pipeline";
        static constexpr const char* kMainWindowService = "window.main";
        static constexpr const char* kNotificationService = "notifications.center";
        static constexpr const char* kProjectServiceKey = "project.service";
        static constexpr const char* kRendererService = "render.main";
        static constexpr const char* kScriptHostService = "script.host";
        static constexpr const char* kSerializationService = "serialization.service";
        static constexpr const char* kTaskSystemService = "task.system";
        static constexpr const char* kTimeSystemService = "time.system";

        static AssetPipeline* s_AssetPipeline;
        static Window* s_MainWindow;
        static NotificationCenter* s_NotificationCenter;
        static ProjectService* s_ProjectService;
        static Renderer* s_Renderer;
        static ScriptHost* s_ScriptHost;
        static SerializationService* s_SerializationService;
        static SDL_Renderer* s_UiRenderer;
        static TaskSystem* s_TaskSystem;
        static TimeSystem* s_TimeSystem;
        static std::unordered_map<std::string, ServiceEntry> s_Services;
        static std::mutex s_ServiceMutex;
    };

} // namespace Nyxty
