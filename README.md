# myvk

Minimal Vulkan framework and examples for testing Vulkan features and building small demos. For a quick overview look at the examples in the _Examples_-folder.

## Clone

    git clone --recurse-submodules -j4 https://github.com/jparimaa/myvk.git

## Build

Run CMake and build the project.

For CMake the following parameters need to be setup

- `ASSIMP_PATH`
- `GLFW_PATH`
- `VULKAN_SDK_PATH` (defaults to vulkan_sdk environment variable)

For example

`cmake . -DASSIMP_PATH=/path/to/assimp_4.1.0 -DGLFW_PATH=/path/to/glfw_3.2.1 -DVULKAN_SDK_PATH=/path/to/VulkanSDK/1.0.61.1/x86_64/ && make`

## Tools

- Vulkan 1.0.61 https://vulkan.lunarg.com/
- GLFW 3.2.1 http://www.glfw.org/download.html
- Assimp 4.1.0 https://github.com/assimp/assimp/releases/tag/v4.1.0/
- GLM 0.9.9.0 (as submodule)
- STB image (as submodule)
- CMake 3.5
- Compiler that supports C++17

Tested on Windows (MSVC 15) and Linux (GCC 7).
