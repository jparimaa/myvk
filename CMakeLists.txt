cmake_minimum_required(VERSION 3.5)
project(myvk-demos VERSION 0.0.1 LANGUAGES CXX)

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "64bit is required")
endif()

set(VULKAN_SDK_PATH $ENV{VULKAN_SDK} CACHE PATH "Path to Vulkan SDK")
set(GLFW_PATH CACHE PATH "Path to GLFW")
set(GLM_PATH CACHE PATH "Path to glm")
set(ASSIMP_PATH CACHE PATH "Path to assimp")
set(STB_PATH CACHE PATH "Path to stb image")
set(ASSETS_PATH CACHE PATH "Path to asset folder")

add_definitions(-DASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/Assets/")

if(WIN32)
    set(VULKAN_LIB_PATH "${VULKAN_SDK_PATH}/Lib")
elseif(UNIX)
    message(FATAL_ERROR "Unix support is insufficient")
else()
    message(FATAL_ERROR "Invalid operating system")
endif()

function(COMPILE_SHADERS)
    file(GLOB_RECURSE GLSL_SOURCE_FILES
        "shaders/*.frag"
        "shaders/*.vert"
    )

    set(GLSL_VALIDATOR "${VULKAN_SDK_PATH}/Bin/glslangValidator.exe")

    foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL}
        )
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach()

    add_custom_target(
        Shaders 
        DEPENDS ${SPIRV_BINARY_FILES}
    )

    add_dependencies(PBR Shaders)

    add_custom_command(TARGET PBR POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:PBR>/shaders/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_BINARY_DIR}/shaders"
            "$<TARGET_FILE_DIR:PBR>/shaders"
    )

    add_definitions(-DSHADER_PATH="$<TARGET_FILE_DIR:PBR>/shaders/")
endfunction()

add_subdirectory(Framework)
add_subdirectory(PBR)
