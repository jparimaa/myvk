add_executable(PBR
    inc/BRDFLUT.h
    inc/EnvironmentImages.h
    inc/Helpers.h
    inc/Offscreen.h
    inc/PBRApp.h
    inc/PipelineHelper.h
    inc/RenderObject.h
    inc/Skybox.h
    src/BRDFLUT.cpp
    src/EnvironmentImages.cpp
    src/main.cpp
    src/Offscreen.cpp
    src/PBRApp.cpp
    src/PipelineHelper.cpp
    src/RenderObject.cpp
    src/Skybox.cpp
)

target_link_libraries(PBR myvk)