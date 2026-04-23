cmake_minimum_required(VERSION 3.20)
project(Core LANGUAGES CXX)

set(ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(THIRD_PARTY_ROOT ${NYXTY_THIRD_PARTY_DIR})
set(NYXTY_BGFX_CMAKE_DIR ${THIRD_PARTY_ROOT}/bgfx.cmake)

set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRD_PARTY_ROOT}/SDL3 ${CMAKE_BINARY_DIR}/third_party/SDL3)

set(BGFX_DIR ${NYXTY_BGFX_CMAKE_DIR}/bgfx CACHE PATH "" FORCE)
set(BX_DIR ${NYXTY_BGFX_CMAKE_DIR}/bx CACHE PATH "" FORCE)
set(BIMG_DIR ${NYXTY_BGFX_CMAKE_DIR}/bimg CACHE PATH "" FORCE)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_TOOLS_SHADER OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_TOOLS_BIN2C OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_TOOLS_GEOMETRY OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_TOOLS_TEXTURE OFF CACHE BOOL "" FORCE)
add_subdirectory(${NYXTY_BGFX_CMAKE_DIR} ${CMAKE_BINARY_DIR}/third_party/bgfx.cmake)

add_subdirectory(${THIRD_PARTY_ROOT}/glm ${CMAKE_BINARY_DIR}/third_party/glm)

set(CORE_FILES
    ${ENGINE_ROOT}/core/foundation/Base.h
    ${ENGINE_ROOT}/core/foundation/Core.h
    ${ENGINE_ROOT}/core/foundation/Core.cpp
    ${ENGINE_ROOT}/core/foundation/Debug.h
    ${ENGINE_ROOT}/core/memory/Memory.h
    ${ENGINE_ROOT}/core/memory/Memory.cpp
    ${ENGINE_ROOT}/core/handles/Handle.h
    ${ENGINE_ROOT}/core/time/TimeSystem.h
    ${ENGINE_ROOT}/core/time/TimeSystem.cpp
    ${ENGINE_ROOT}/core/logging/Log.h
    ${ENGINE_ROOT}/core/logging/Log.cpp
    ${ENGINE_ROOT}/core/logging/LogSink.h
    ${ENGINE_ROOT}/core/events/Event.h
    ${ENGINE_ROOT}/core/events/Event.cpp
    ${ENGINE_ROOT}/core/events/EventBus.h
    ${ENGINE_ROOT}/core/events/EventBus.cpp
    ${ENGINE_ROOT}/core/events/EventBusInstance.h
    ${ENGINE_ROOT}/core/events/EventBusInstance.cpp
    ${ENGINE_ROOT}/core/events/EventTypes.h
    ${ENGINE_ROOT}/core/events/EventTypes.cpp
    ${ENGINE_ROOT}/services/container/ServiceLocator.h
    ${ENGINE_ROOT}/services/container/ServiceLocator.cpp
    ${ENGINE_ROOT}/services/window/Window.h
    ${ENGINE_ROOT}/services/window/Window.cpp
    ${ENGINE_ROOT}/services/filesystem/FileWatcher.h
    ${ENGINE_ROOT}/services/filesystem/FileWatcher.cpp
    ${ENGINE_ROOT}/services/assets/AssetPipeline.h
    ${ENGINE_ROOT}/services/assets/AssetPipeline.cpp
    ${ENGINE_ROOT}/services/config/Config.h
    ${ENGINE_ROOT}/services/config/Config.cpp
    ${ENGINE_ROOT}/services/config/BuildProfile.h
    ${ENGINE_ROOT}/services/config/BuildProfile.cpp
    ${ENGINE_ROOT}/services/serialization/SerializationService.h
    ${ENGINE_ROOT}/services/serialization/SerializationService.cpp
    ${ENGINE_ROOT}/services/tasks/TaskSystem.h
    ${ENGINE_ROOT}/services/tasks/TaskSystem.cpp
    ${ENGINE_ROOT}/services/notifications/NotificationCenter.h
    ${ENGINE_ROOT}/services/notifications/NotificationCenter.cpp
    ${ENGINE_ROOT}/services/project/Project.h
    ${ENGINE_ROOT}/services/project/Project.cpp
    ${ENGINE_ROOT}/rendering/backend/Renderer.h
    ${ENGINE_ROOT}/rendering/backend/Renderer.cpp
    ${ENGINE_ROOT}/scripting/runtime/ScriptHost.h
    ${ENGINE_ROOT}/scripting/runtime/ScriptHost.cpp
)

add_library(core SHARED ${CORE_FILES})

source_group(TREE "${ENGINE_ROOT}" FILES ${CORE_FILES})

target_compile_definitions(core PRIVATE NYXTY_CORE_BUILD)

target_include_directories(core
    PUBLIC
        ${ENGINE_ROOT}
        ${THIRD_PARTY_ROOT}/spdlog/include
        ${THIRD_PARTY_ROOT}/entt/src
        ${THIRD_PARTY_ROOT}/json
        ${THIRD_PARTY_ROOT}/glm
        ${NYXTY_BGFX_CMAKE_DIR}/bgfx/include
        ${NYXTY_BGFX_CMAKE_DIR}/bx/include
        ${NYXTY_BGFX_CMAKE_DIR}/bimg/include
)

target_link_libraries(core
    PUBLIC
        platform
        SDL3::SDL3
        bgfx
        glm::glm
)

if(WIN32)
    target_compile_definitions(core PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS)
    target_include_directories(core PRIVATE ${NYXTY_BGFX_CMAKE_DIR}/bx/include/compat/msvc)
    target_link_libraries(core PRIVATE gdi32 psapi d3d11 dxgi)
elseif(APPLE)
    target_link_libraries(core PRIVATE
        "-framework Cocoa"
        "-framework Metal"
        "-framework QuartzCore")
else()
    target_link_libraries(core PRIVATE GL X11 pthread dl)
endif()

if(MSVC)
    target_compile_options(core PRIVATE /utf-8 /FS)
endif()

set_target_properties(core PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${NYXTY_RUNTIME_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${NYXTY_RUNTIME_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${NYXTY_RUNTIME_OUTPUT_DIR}"
)

add_custom_command(TARGET core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:core>
        "${NYXTY_RUNTIME_OUTPUT_DIR}/$<TARGET_FILE_NAME:core>"
    COMMENT "Copying core runtime library"
)
