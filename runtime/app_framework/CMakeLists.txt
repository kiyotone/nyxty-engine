cmake_minimum_required(VERSION 3.20)
project(AppFramework LANGUAGES CXX)

add_library(app_framework STATIC
    AppRunner.cpp
    AppRunner.h
    IApp.h
)

target_include_directories(app_framework
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../..   # engine repo root — stable whether standalone or submodule
)

if(MSVC)
    target_compile_options(app_framework PRIVATE /utf-8)
endif()

set_target_properties(app_framework PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${NYXTY_RUNTIME_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${NYXTY_RUNTIME_OUTPUT_DIR}"
)
