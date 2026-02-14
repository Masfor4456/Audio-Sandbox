cmake_minimum_required(VERSION 3.18)
project(AudioSandbox)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# Source Files
# ============================================================================

set(AUDIO_SOURCES
    Source/Audio/AudioSynthesizer.cpp
    Source/Audio/AudioSynthesizer.h
)

set(PHYSICS_SOURCES
    Source/Physics/PhysicsCore.cpp
    Source/Physics/PhysicsCore.h
)

set(INTEGRATION_SOURCES
    Source/Integration/AudioPhysicsIntegration.cpp
    Source/Integration/AudioPhysicsIntegration.h
)

set(PROCEDURAL_SOURCES
    Source/Procedural/ProceduralGeneration.cpp
    Source/Procedural/ProceduralGeneration.h
)

set(CORE_SOURCES
    Source/SandboxManager.cpp
    Source/SandboxManager.h
)

set(EXAMPLE_SOURCES
    Source/Examples.cpp
)

# Create library
add_library(AudioSandbox STATIC
    ${AUDIO_SOURCES}
    ${PHYSICS_SOURCES}
    ${INTEGRATION_SOURCES}
    ${PROCEDURAL_SOURCES}
    ${CORE_SOURCES}
)

# Include directories
target_include_directories(AudioSandbox PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# Create executable for examples
add_executable(AudioSandboxExamples ${EXAMPLE_SOURCES})

target_link_libraries(AudioSandboxExamples
    AudioSandbox
)

target_include_directories(AudioSandboxExamples PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# ============================================================================
# Compiler Flags
# ============================================================================

if(MSVC)
    target_compile_options(AudioSandbox PRIVATE /W4 /WX)
    target_compile_options(AudioSandboxExamples PRIVATE /W4 /WX)
else()
    target_compile_options(AudioSandbox PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(AudioSandboxExamples PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ============================================================================
# Installation
# ============================================================================

install(TARGETS AudioSandbox
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY Source/
    DESTINATION include/AudioSandbox
    FILES_MATCHING PATTERN "*.h"
)

# ============================================================================
# Testing (optional)
# ============================================================================

enable_testing()

# Test executable
add_executable(AudioSandboxTests 
    Tests/TestSynthesis.cpp
    Tests/TestPhysics.cpp
    Tests/TestIntegration.cpp
)

target_link_libraries(AudioSandboxTests AudioSandbox)
target_include_directories(AudioSandboxTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)

add_test(NAME AudioSandboxTests COMMAND AudioSandboxTests)

# ============================================================================
# Summary
# ============================================================================

message(STATUS "AudioSandbox Configuration Summary:")
message(STATUS "  Generator: ${CMAKE_GENERATOR}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "")
message(STATUS "Targets:")
message(STATUS "  - AudioSandbox (library)")
message(STATUS "  - AudioSandboxExamples (executable)")
message(STATUS "  - AudioSandboxTests (tests)")
message(STATUS "")
message(STATUS "To build:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake -DCMAKE_BUILD_TYPE=Release ..")
message(STATUS "  cmake --build . --config Release")
message(STATUS "  ./bin/AudioSandboxExamples")
