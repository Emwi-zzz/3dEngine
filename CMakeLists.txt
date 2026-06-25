cmake_minimum_required(VERSION 3.14)
project(3dEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

# Assimp
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v5.4.1
)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(assimp)

# Glad (generated via pip in third_party)
add_library(glad third_party/glad/src/gl.c)
target_include_directories(glad PUBLIC third_party/glad/include)

# Executable
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(3dEngine ${SOURCES})

# Compile Definitions
target_compile_definitions(3dEngine PRIVATE ENABLE_UNDERWATER_ENVIRONMENT)
target_compile_definitions(3dEngine PRIVATE ENABLE_ENDLESS_FLOOR)

# Includes and Linking
target_include_directories(3dEngine PRIVATE src)
target_link_libraries(3dEngine PRIVATE glfw glm::glm glad assimp ${CMAKE_DL_LIBS})
