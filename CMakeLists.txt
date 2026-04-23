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
set(NYXTY_THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party" CACHE PATH "Nyxty third-party dependency directory")

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
add_subdirectory(apps/hub)
