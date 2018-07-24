add_executable(PBR
    include/BRDFLUT.h
    include/EnvironmentImages.h
    include/Helpers.h
    include/Offscreen.h
    include/PBRApp.h
    include/PipelineHelper.h
    include/RenderObject.h
    include/Skybox.h
    src/BRDFLUT.cpp
    src/EnvironmentImages.cpp
    src/main.cpp
    src/Offscreen.cpp
    src/PBRApp.cpp
    src/PipelineHelper.cpp
    src/RenderObject.cpp
    src/Skybox.cpp
)

target_link_libraries(PBR 
    PRIVATE
        myvk
)

target_include_directories(PBR
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Framework/include"
)

add_custom_command(TARGET PBR POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ASSIMP_PATH}/bin/x64/assimp-vc140-mt.dll"
        $<TARGET_FILE_DIR:PBR>
)

# Shaders
message(${CMAKE_HOST_SYSTEM_PROCESSOR})

if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
  set(GLSL_VALIDATOR "${VULKAN_SDK_PATH}/Bin/glslangValidator.exe")
else()
  set(GLSL_VALIDATOR "${VULKAN_SDK_PATH}/Bin32/glslangValidator.exe")
endif()

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "shaders/*.frag"
    "shaders/*.vert"
)

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