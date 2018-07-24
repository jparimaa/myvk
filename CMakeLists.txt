cmake_minimum_required(VERSION 3.5)
project(myvk-demos VERSION 0.0.1 LANGUAGES CXX)

set(VULKAN_SDK_PATH $ENV{VULKAN_SDK} CACHE PATH "Path to Vulkan SDK")
set(GLFW_PATH CACHE PATH "Path to GLFW")
set(GLM_PATH CACHE PATH "Path to glm")
set(ASSIMP_PATH CACHE PATH "Path to assimp")
set(STB_PATH CACHE PATH "Path to stb image")
set(ASSETS_PATH CACHE PATH "Path to asset folder")

add_definitions(-DASSETS_PATH="${ASSETS_PATH}")

if(WIN32)
    set(VULKAN_LIB_PATH "${VULKAN_SDK_PATH}/Lib")
elseif(UNIX)
    message(FATAL_ERROR "Unix support is insufficient")
else()
    message(FATAL_ERROR "Invalid operating system")
endif()

add_subdirectory(Framework)
add_subdirectory(PBR)