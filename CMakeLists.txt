cmake_minimum_required(VERSION 3.20)
project(Procedurking)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "C:/Users/Pzod/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Find packages using vcpkg
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)

add_executable(Procedurking 
    src/main.cpp
    src/Shader.cpp
    src/Icosphere.cpp
    src/Camera.cpp
    src/TerrainSampler.cpp
    src/Starfield.cpp
    src/PlanetaryRings.cpp
    src/CelestialBody.cpp
    src/PlanetManager.cpp
    src/LifeForm.cpp
)

target_include_directories(Procedurking PRIVATE
    libs/FastNoiseLite
    src/
    libs/FastNoiseLite/Cpp
)

target_link_libraries(Procedurking PRIVATE
    glfw
    glad::glad
    glm::glm
)

file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR}) 