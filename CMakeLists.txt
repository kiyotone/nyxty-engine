cmake_minimum_required(VERSION 3.20)
project(Nyxty LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(NYXTY_ENABLE_ENGINE_INSPECTOR "Build the reusable imgui-based engine inspector UI" ON)

# ========================================
# Platform Defines
# ========================================
if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
elseif(UNIX)
    add_definitions(-DPLATFORM_LINUX)
endif()

set(NYXTY_RUNTIME_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Nyxty runtime output directory")
set(NYXTY_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party" CACHE PATH "Nyxty third-party dependency directory")

option(NYXTY_BUILD_SHADER_TOOLS "Build bgfx shaderc for shader compilation" OFF)
if(NYXTY_BUILD_SHADER_TOOLS)
    set(BGFX_BUILD_TOOLS ON CACHE BOOL "" FORCE)
    set(BGFX_BUILD_TOOLS_SHADER ON CACHE BOOL "" FORCE)
endif()

# ========================================
# Engine
# ========================================
add_subdirectory(engine)

# ========================================
# Runtime
# ========================================
add_subdirectory(runtime)

# ========================================
# Apps
# ========================================
# Apps are standalone projects that consume this engine as a git submodule.
# They are NOT built here. See: https://github.com/kiyotone/nyxel
#
# To add an app that uses this engine:
#   git submodule add <nyxty-engine-repo> engine
#   add_subdirectory(engine)
#   target_link_libraries(your_app PRIVATE app_framework modsys core engine)
