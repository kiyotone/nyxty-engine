cmake_minimum_required(VERSION 3.20)
project(ModuleSystem LANGUAGES CXX)

# Build ModuleSystem as SHARED library
add_library(modsys SHARED
    ModuleLoader.cpp
    ModuleLoader.h
    IModule.h
)

# Define macro when building DLL
target_compile_definitions(modsys PRIVATE NYXTY_MODULE_BUILD)

# Include directories
target_include_directories(modsys
    PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/engine
        ${NYXTY_THIRD_PARTY_DIR}/spdlog/include
        ${NYXTY_THIRD_PARTY_DIR}/entt/src
        ${NYXTY_THIRD_PARTY_DIR}/json
)

# Module depends on Core + Platform
target_link_libraries(modsys
    PUBLIC
        core
        platform
)

# Fix for spdlog/fmt Unicode support on MSVC
if(MSVC)
    target_compile_options(modsys PRIVATE /utf-8)
endif()

# Ensure DLL + import library go to app folder
set_target_properties(modsys
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${NYXTY_RUNTIME_OUTPUT_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${NYXTY_RUNTIME_OUTPUT_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${NYXTY_RUNTIME_OUTPUT_DIR}"
)

# Copy only DLL explicitly
add_custom_command(TARGET modsys POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:modsys>
        "${NYXTY_RUNTIME_OUTPUT_DIR}/$<TARGET_FILE_NAME:modsys>"
    COMMENT "Copying module system runtime library"
)
