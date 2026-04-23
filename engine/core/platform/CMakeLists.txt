cmake_minimum_required(VERSION 3.16)
project(Platform LANGUAGES CXX)

add_library(platform SHARED OS.cpp)
target_compile_definitions(platform PRIVATE NYXTY_PLATFORM_BUILD)
target_include_directories(platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# Ensure DLL and LIB go to same folder
set_target_properties(platform
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${NYXTY_RUNTIME_OUTPUT_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${NYXTY_RUNTIME_OUTPUT_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${NYXTY_RUNTIME_OUTPUT_DIR}"
)

# Copy only DLL (LIB is already in ARCHIVE_OUTPUT_DIRECTORY)
add_custom_command(TARGET platform POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:platform>
        "${NYXTY_RUNTIME_OUTPUT_DIR}/$<TARGET_FILE_NAME:platform>"
    COMMENT "Copying platform runtime library"
)
