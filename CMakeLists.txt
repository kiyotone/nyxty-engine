cmake_minimum_required(VERSION 3.20)
project(Nyxty LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
elseif(UNIX)
    add_definitions(-DPLATFORM_LINUX)
endif()

set(NYXTY_ROOT_DIR           "${CMAKE_CURRENT_SOURCE_DIR}"             CACHE PATH "Nyxty repo root")
set(NYXTY_RUNTIME_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin"                 CACHE PATH "Runtime output dir")
set(NYXTY_THIRD_PARTY_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/third_party" CACHE PATH "Third-party dir")

add_subdirectory(${NYXTY_THIRD_PARTY_DIR}/glm ${CMAKE_BINARY_DIR}/third_party/glm)
add_subdirectory(core)
