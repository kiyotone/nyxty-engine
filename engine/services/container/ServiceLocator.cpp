#include "services/container/ServiceLocator.h"

#include "services/assets/AssetPipeline.h"
#include "services/notifications/NotificationCenter.h"
#include "services/project/Project.h"
#include "scripting/runtime/ScriptHost.h"
#include "services/serialization/SerializationService.h"
#include "services/tasks/TaskSystem.h"
#include "core/time/TimeSystem.h"

namespace Nyxty {

AssetPipeline* ServiceLocator::s_AssetPipeline = nullptr;
Window* ServiceLocator::s_MainWindow = nullptr;
NotificationCenter* ServiceLocator::s_NotificationCenter = nullptr;
ProjectService* ServiceLocator::s_ProjectService = nullptr;
Renderer* ServiceLocator::s_Renderer = nullptr;
ScriptHost* ServiceLocator::s_ScriptHost = nullptr;
SerializationService* ServiceLocator::s_SerializationService = nullptr;
SDL_Renderer* ServiceLocator::s_UiRenderer = nullptr;
TaskSystem* ServiceLocator::s_TaskSystem = nullptr;
TimeSystem* ServiceLocator::s_TimeSystem = nullptr;
std::unordered_map<std::string, ServiceLocator::ServiceEntry> ServiceLocator::s_Services;
std::mutex ServiceLocator::s_ServiceMutex;

void ServiceLocator::Provide(Window* mainWindow, Renderer* renderer, SDL_Renderer* uiRenderer) {
    s_MainWindow = mainWindow;
    s_Renderer = renderer;
    s_UiRenderer = uiRenderer;

    Register<Window>(kMainWindowService, mainWindow);
    Register<Renderer>(kRendererService, renderer);
}

void ServiceLocator::ProvideAssets(AssetPipeline* assetPipeline) {
    s_AssetPipeline = assetPipeline;
    Register<AssetPipeline>(kAssetPipelineService, assetPipeline);
}

void ServiceLocator::ProvideNotifications(NotificationCenter* notificationCenter) {
    s_NotificationCenter = notificationCenter;
    Register<NotificationCenter>(kNotificationService, notificationCenter);
}

void ServiceLocator::ProvideProject(ProjectService* projectService) {
    s_ProjectService = projectService;
    Register<ProjectService>(kProjectServiceKey, projectService);
}

void ServiceLocator::ProvideScripting(ScriptHost* scriptHost) {
    s_ScriptHost = scriptHost;
    Register<ScriptHost>(kScriptHostService, scriptHost);
}

void ServiceLocator::ProvideSerialization(SerializationService* serializationService) {
    s_SerializationService = serializationService;
    Register<SerializationService>(kSerializationService, serializationService);
}

void ServiceLocator::ProvideTasks(TaskSystem* taskSystem) {
    s_TaskSystem = taskSystem;
    Register<TaskSystem>(kTaskSystemService, taskSystem);
}

void ServiceLocator::ProvideTime(TimeSystem* timeSystem) {
    s_TimeSystem = timeSystem;
    Register<TimeSystem>(kTimeSystemService, timeSystem);
}

bool ServiceLocator::Has(const std::string& key) {
    std::scoped_lock lock(s_ServiceMutex);
    return s_Services.contains(key);
}

void ServiceLocator::Unregister(const std::string& key) {
    std::scoped_lock lock(s_ServiceMutex);
    s_Services.erase(key);
}

void ServiceLocator::Clear() {
    std::scoped_lock lock(s_ServiceMutex);
    s_Services.clear();
    s_AssetPipeline = nullptr;
    s_MainWindow = nullptr;
    s_NotificationCenter = nullptr;
    s_ProjectService = nullptr;
    s_Renderer = nullptr;
    s_ScriptHost = nullptr;
    s_SerializationService = nullptr;
    s_UiRenderer = nullptr;
    s_TaskSystem = nullptr;
    s_TimeSystem = nullptr;
}

Window* ServiceLocator::GetMainWindow() {
    return s_MainWindow ? s_MainWindow : Get<Window>(kMainWindowService);
}

AssetPipeline* ServiceLocator::GetAssetPipeline() {
    return s_AssetPipeline ? s_AssetPipeline : Get<AssetPipeline>(kAssetPipelineService);
}

NotificationCenter* ServiceLocator::GetNotificationCenter() {
    return s_NotificationCenter ? s_NotificationCenter : Get<NotificationCenter>(kNotificationService);
}

ProjectService* ServiceLocator::GetProjectService() {
    return s_ProjectService ? s_ProjectService : Get<ProjectService>(kProjectServiceKey);
}

Renderer* ServiceLocator::GetRenderer() {
    return s_Renderer ? s_Renderer : Get<Renderer>(kRendererService);
}

ScriptHost* ServiceLocator::GetScriptHost() {
    return s_ScriptHost ? s_ScriptHost : Get<ScriptHost>(kScriptHostService);
}

SerializationService* ServiceLocator::GetSerializationService() {
    return s_SerializationService ? s_SerializationService : Get<SerializationService>(kSerializationService);
}

SDL_Renderer* ServiceLocator::GetUiRenderer() {
    return s_UiRenderer;
}

TaskSystem* ServiceLocator::GetTaskSystem() {
    return s_TaskSystem ? s_TaskSystem : Get<TaskSystem>(kTaskSystemService);
}

TimeSystem* ServiceLocator::GetTimeSystem() {
    return s_TimeSystem ? s_TimeSystem : Get<TimeSystem>(kTimeSystemService);
}

} // namespace Nyxty
