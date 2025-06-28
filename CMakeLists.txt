# Project Genesis: The Shattering - Voxel Destruction Engine
cmake_minimum_required(VERSION 3.20)
project(ProcedurKing VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options (removed conflicting optimization flags)
if(MSVC)
    add_compile_options(/W4 /openmp)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -fopenmp)
endif()

# Find packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Set up vcpkg
set(CMAKE_TOOLCHAIN_FILE "C:/Users/Pzod/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Find vcpkg packages
find_package(glad CONFIG REQUIRED)

# FastNoiseLite is header-only, no need for add_subdirectory
# Just include the header directory

# Source files
set(SOURCES
    # Original core files
    src/main.cpp
    src/Camera.cpp
    src/Shader.cpp
    src/Icosphere.cpp
    src/Starfield.cpp
    src/TerrainSampler.cpp
    src/CelestialBody.cpp
    src/PlanetaryRings.cpp
    src/PlanetManager.cpp
    src/GameState.cpp
    src/LifeForm.cpp
    src/PlanetarySystem.cpp
    src/GodPowers.cpp
    
    # New voxel system core
    src/VoxelCore.cpp
    src/VoxelWorldManager.cpp
    # src/VoxelPlanetGenerator.cpp
    # src/VoxelMesher.cpp
    # src/DestructionManager.cpp
    # src/StructuralIntegrityManager.cpp
    # src/VoxelClimateSimulator.cpp
    # src/VoxelAIDirector.cpp
    # src/VoxelShatteringEngine.cpp
)

set(HEADERS
    # Original headers
    src/Camera.h
    src/Shader.h
    src/Icosphere.h
    src/Starfield.h
    src/TerrainSampler.h
    src/CelestialBody.h
    src/PlanetaryRings.h
    src/PlanetManager.h
    src/GameState.h
    src/LifeForm.h
    src/PlanetarySystem.h
    src/GodPowers.h
    
    # New voxel system headers
    src/VoxelCore.h
    src/VoxelWorldManager.h
    src/VoxelPlanetGenerator.h
    src/VoxelMesher.h
    src/DestructionManager.h
    src/StructuralIntegrityManager.h
    src/VoxelClimateSimulator.h
    src/VoxelAIDirector.h
    src/VoxelShatteringEngine.h
)

# Shader files
set(SHADERS
    shaders/planet.vert
    shaders/planet.frag
    shaders/atmosphere.vert
    shaders/atmosphere.frag
    shaders/stars.vert
    shaders/stars.frag
    shaders/rings.vert
    shaders/rings.frag
    shaders/clouds.vert
    shaders/clouds.frag
    shaders/moon.frag
    
    # New voxel shaders (to be created)
    # shaders/voxel.vert
    # shaders/voxel.frag
    # shaders/voxel_dual_contouring.comp
    # shaders/debris.vert
    # shaders/debris.frag
    # shaders/particles.vert
    # shaders/particles.frag
    # shaders/debug.vert
    # shaders/debug.frag
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    src/
    libs/FastNoiseLite/
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
    glad::glad
    glm::glm
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE GL pthread)
endif()

# Copy shaders to build directory
foreach(SHADER ${SHADERS})
    configure_file(${CMAKE_SOURCE_DIR}/${SHADER} ${CMAKE_BINARY_DIR}/${SHADER} COPYONLY)
endforeach()

# Set working directory for Visual Studio
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

# Add custom target for shader compilation validation
add_custom_target(validate_shaders
    COMMENT "Validating shader files..."
)

# Performance optimization flags (only for Release builds)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /O2 /Ob2 /Oi /Ot /GL /arch:AVX2)
        target_link_options(${PROJECT_NAME} PRIVATE /LTCG)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -O3 -flto -ffast-math -march=native)
        target_link_options(${PROJECT_NAME} PRIVATE -flto)
    endif()
    
    # Enable aggressive optimizations for voxel systems
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        VOXEL_OPTIMIZED_BUILD=1
        ENABLE_SIMD_OPTIMIZATIONS=1
        ENABLE_GPU_ACCELERATION=1
    )
endif()

# Debug configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        VOXEL_DEBUG_BUILD=1
        ENABLE_VOXEL_VALIDATION=1
        ENABLE_PERFORMANCE_PROFILING=1
    )
endif()

# Feature flags for voxel system
target_compile_definitions(${PROJECT_NAME} PRIVATE
    VOXEL_CHUNK_SIZE=64
    VOXEL_MAX_WORLD_SIZE=1048576
    VOXEL_ENABLE_COMPRESSION=1
    VOXEL_ENABLE_MULTITHREADING=1
    VOXEL_ENABLE_GPU_COMPUTE=1
    DESTRUCTION_ENABLE_CHAIN_REACTIONS=1
    STRUCTURAL_ENABLE_FATIGUE_SIMULATION=1
    CLIMATE_ENABLE_FIRE_SIMULATION=1
    CLIMATE_ENABLE_FLUID_SIMULATION=1
    AI_DIRECTOR_ENABLE_NARRATIVE_SYSTEM=1
)

# Memory management settings
target_compile_definitions(${PROJECT_NAME} PRIVATE
    VOXEL_MAX_ACTIVE_CHUNKS=1000
    VOXEL_MAX_DEBRIS_OBJECTS=10000
    VOXEL_MEMORY_POOL_SIZE_MB=2048
)

# Platform-specific voxel optimizations
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        VOXEL_USE_WINDOWS_THREADING=1
        VOXEL_USE_DIRECTX_INTEROP=0  # OpenGL only for now
    )
endif()

# Add source groups for IDE organization
source_group("Source Files\\Core" FILES ${SOURCES})
source_group("Header Files\\Core" FILES ${HEADERS})
source_group("Shaders" FILES ${SHADERS})

# Voxel system source groups
source_group("Header Files\\Voxel\\Core" FILES 
    src/VoxelCore.h
    src/VoxelWorldManager.h
)
source_group("Header Files\\Voxel\\Generation" FILES 
    src/VoxelPlanetGenerator.h
    src/VoxelMesher.h
)
source_group("Header Files\\Voxel\\Physics" FILES 
    src/DestructionManager.h
    src/StructuralIntegrityManager.h
)
source_group("Header Files\\Voxel\\Simulation" FILES 
    src/VoxelClimateSimulator.h
    src/VoxelAIDirector.h
)
source_group("Header Files\\Voxel\\Engine" FILES 
    src/VoxelShatteringEngine.h
)

# Print configuration summary
message(STATUS "=== Project Genesis: The Shattering Configuration ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "OpenGL Version: ${OPENGL_VERSION}")
message(STATUS "GLFW Version: ${glfw3_VERSION}")
message(STATUS "Voxel System: ENABLED")
message(STATUS "Destruction Physics: ENABLED")
message(STATUS "Structural Simulation: ENABLED")
message(STATUS "Climate Simulation: ENABLED")
message(STATUS "AI Director: ENABLED")
message(STATUS "GPU Acceleration: ${VOXEL_ENABLE_GPU_COMPUTE}")
message(STATUS "Multi-threading: ${VOXEL_ENABLE_MULTITHREADING}")
message(STATUS "=============================================")

# Add installation rules (optional)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY shaders/
    DESTINATION bin/shaders
    FILES_MATCHING PATTERN "*.vert" PATTERN "*.frag" PATTERN "*.comp"
) 