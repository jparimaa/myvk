cmake_minimum_required(VERSION 3.5)
project(myvk-demos VERSION 0.0.1 LANGUAGES CXX)

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "64bit is required")
endif()

set(VULKAN_SDK_PATH $ENV{VULKAN_SDK} CACHE PATH "Path to Vulkan SDK")
set(GLFW_PATH CACHE PATH "Path to GLFW")
set(ASSIMP_PATH CACHE PATH "Path to assimp")

set(STB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/External/stb/")
set(GLM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/External/glm/")

add_definitions(-DASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/Assets/")

if(WIN32)
    set(VULKAN_LIB_PATH "${VULKAN_SDK_PATH}/Lib")
    set(GLSL_VALIDATOR "${VULKAN_SDK_PATH}/Bin/glslangValidator.exe")
elseif(UNIX)
    set(VULKAN_LIB_PATH "${VULKAN_SDK_PATH}/lib")
    set(GLSL_VALIDATOR "${VULKAN_SDK_PATH}/bin/glslangValidator")
else()
    message(FATAL_ERROR "Invalid operating system")
endif()

function(ADD_PROJECT_WITH_DEFAULT_SETTINGS PROJECT_NAME)
    file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

    add_executable(${PROJECT_NAME} ${SOURCES})

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            myvk
    )

    target_include_directories(${PROJECT_NAME}
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../Framework/include"
    )

    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX /MP)
        target_compile_definitions(${PROJECT_NAME} PUBLIC -D_CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
    endif()

    if(WIN32)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ASSIMP_PATH}/bin/x64/assimp-vc140-mt.dll"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )
    endif()
endfunction()

function(COMPILE_SHADERS MAIN_PROJECT SHADER_PROJECT)
    file(GLOB_RECURSE GLSL_SOURCE_FILES
        "shaders/*.frag"
        "shaders/*.vert"
        "shaders/*.comp"
    )

    foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        set(SHADER_FOLDER "${PROJECT_BINARY_DIR}/Shaders/${MAIN_PROJECT}")
        set(SPIRV "${SHADER_FOLDER}/${FILE_NAME}.spv")
        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_FOLDER}"
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL}
        )
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach()

    add_custom_target(
        ${SHADER_PROJECT}
        DEPENDS ${SPIRV_BINARY_FILES}
    )

    add_dependencies(${MAIN_PROJECT} ${SHADER_PROJECT} )
    
    add_definitions(-DSHADER_PATH="${SHADER_FOLDER}/")
endfunction()

add_subdirectory(Framework)
add_subdirectory(Examples/Minimal)
add_subdirectory(Examples/PBR)
add_subdirectory(Examples/Subpass)
add_subdirectory(Examples/SecondaryCommandBuffer)
add_subdirectory(Examples/DynamicUniformBuffer)
add_subdirectory(Examples/PushConstant)
add_subdirectory(Examples/SpecializationConstant)
add_subdirectory(Examples/Triangle)
add_subdirectory(Examples/LightShaft)
add_subdirectory(Examples/Reflection)
add_subdirectory(Examples/Multisampling)
add_subdirectory(Examples/Mandelbrot)
add_subdirectory(Examples/Particles)
add_subdirectory(Examples/Clustered)