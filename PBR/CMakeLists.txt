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
        "${ASSIMP_PATH}/bin/x86/assimp-vc140-mt.dll"
        $<TARGET_FILE_DIR:PBR>
)