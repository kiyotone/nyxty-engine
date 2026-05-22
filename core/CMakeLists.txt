cmake_minimum_required(VERSION 3.20)
project(NyxtyCore LANGUAGES CXX)

set(CORE_SOURCES
    # Foundation
    foundation/Core.cpp
    # Platform
    platform/OS.cpp
    # Events
    events/Event.cpp
    events/EventBus.cpp
    events/EventBusInstance.cpp
    events/EventTypes.cpp
    # Logging
    logging/Log.cpp
    # Memory
    memory/Memory.cpp
    # Time
    time/TimeSystem.cpp
    # Jobs
    jobs/TaskSystem.cpp
    # Config
    config/Config.cpp
    config/BuildProfile.cpp
    # IO
    io/FileWatcher.cpp
    io/SerializationService.cpp
    # Services
    services/ServiceLocator.cpp
    services/NotificationCenter.cpp
    services/AssetPipeline.cpp
    services/Project.cpp
    # Runtime
    runtime/AppRunner.cpp
    runtime/ModuleLoader.cpp
)

add_library(core SHARED ${CORE_SOURCES})

target_compile_definitions(core PRIVATE NYXTY_CORE_BUILD)

target_include_directories(core
    PUBLIC
        ${NYXTY_ROOT_DIR}                       # repo root: "core/...", "interfaces/..."
        ${NYXTY_THIRD_PARTY_DIR}/spdlog/include  # spdlog
        ${NYXTY_THIRD_PARTY_DIR}/entt/src        # entt delegates
        ${NYXTY_THIRD_PARTY_DIR}/json            # nlohmann json
)

target_link_libraries(core
    PUBLIC
        glm::glm
)

# Platform-specific links
if(WIN32)
    target_compile_definitions(core PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS)
elseif(APPLE)
    # nothing extra needed for core on macOS
elseif(UNIX)
    target_link_libraries(core PRIVATE pthread dl)
endif()

if(MSVC)
    target_compile_options(core PRIVATE /utf-8 /FS)
endif()

set_target_properties(core PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${NYXTY_RUNTIME_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${NYXTY_RUNTIME_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${NYXTY_RUNTIME_OUTPUT_DIR}"
)
