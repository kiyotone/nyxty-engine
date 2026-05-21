cmake_minimum_required(VERSION 3.20)
project(NyxtyEngine LANGUAGES CXX)

# -----------------------------
# Third-party include paths (header-only)
# -----------------------------
set(SPDLOG_INCLUDE ${NYXTY_THIRD_PARTY_DIR}/spdlog/include)
set(ENTT_INCLUDE ${NYXTY_THIRD_PARTY_DIR}/entt/src)
set(JSON_INCLUDE ${NYXTY_THIRD_PARTY_DIR}/json)

# -----------------------------
# Add runtime libraries.
# Order matters: core/platform -> core.
# -----------------------------
add_subdirectory(core/platform)     # platform.dll
add_subdirectory(core)              # core.dll

# -----------------------------
# Engine INTERFACE library
# -----------------------------
add_library(engine INTERFACE)

# Expose folder-style includes from the engine root.
target_include_directories(engine
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}            # engine root
        ${SPDLOG_INCLUDE}
        ${ENTT_INCLUDE}
        ${JSON_INCLUDE}
)

# Link DLLs to Engine
target_link_libraries(engine
    INTERFACE
        platform
        core
)

# -----------------------------
# Engine version
# -----------------------------
set(NYXTY_ENGINE_VERSION_MAJOR 1)
set(NYXTY_ENGINE_VERSION_MINOR 3)
set(NYXTY_ENGINE_VERSION_PATCH 0)

target_compile_definitions(engine
    INTERFACE
        NYXTY_ENGINE_VERSION_MAJOR=${NYXTY_ENGINE_VERSION_MAJOR}
        NYXTY_ENGINE_VERSION_MINOR=${NYXTY_ENGINE_VERSION_MINOR}
        NYXTY_ENGINE_VERSION_PATCH=${NYXTY_ENGINE_VERSION_PATCH}
)
