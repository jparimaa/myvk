cmake_minimum_required(VERSION 3.5)
project(myvk-fw VERSION 1.0.0 LANGUAGES CXX)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.12")
        message(FATAL_ERROR "Insufficient compiler version, version 19.12 in requires (VS 15 2017)")
    endif()
endif()

add_library(myvk
    include/fw/API.h
    include/fw/Application.h
    include/fw/Buffer.h
    include/fw/Camera.h
    include/fw/CameraController.h
    include/fw/Command.h
    include/fw/Common.h
    include/fw/Constants.h
    include/fw/Context.h
    include/fw/Device.h
    include/fw/Execute.h
    include/fw/Framework.h
    include/fw/GUI.h
    include/fw/Image.h
    include/fw/Input.h
    include/fw/Instance.h
    include/fw/Macros.h
    include/fw/Mesh.h
    include/fw/Model.h
    include/fw/Pipeline.h
    include/fw/RenderPass.h
    include/fw/Sampler.h
    include/fw/SwapChain.h
    include/fw/Texture.h
    include/fw/Time.h
    include/fw/Transformation.h
    include/fw/Window.h
    src/API.cpp
    src/Buffer.cpp
    src/Camera.cpp
    src/CameraController.cpp
    src/Command.cpp
    src/Common.cpp
    src/Context.cpp
    src/Device.cpp
    src/Framework.cpp
    src/GUI.cpp
    src/Image.cpp
    src/Input.cpp
    src/Instance.cpp
    src/Mesh.cpp
    src/Model.cpp
    src/Pipeline.cpp
    src/RenderPass.cpp
    src/Sampler.cpp
    src/SwapChain.cpp
    src/Texture.cpp
    src/Time.cpp
    src/Transformation.cpp
    src/Window.cpp
    imgui/imgui.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_impl_glfw_vulkan.cpp
)

if (WIN32)
    target_link_libraries(myvk
        PUBLIC
            "${VULKAN_LIB_PATH}/vulkan-1.lib"
            "${GLFW_PATH}/lib-vc2015/glfw3.lib"
            "${ASSIMP_PATH}/lib/x64/assimp-vc140-mt.lib"
    )
elseif (UNIX)
    target_link_libraries(myvk
        PUBLIC
            "${GLFW_PATH}/src/libglfw3.a"
            "${VULKAN_LIB_PATH}/libvulkan.so"
            "${ASSIMP_PATH}/lib/libassimp.so"
            "rt" "m" "dl" "Xrandr" "Xinerama" "Xxf86vm" "Xext" "Xcursor" "Xrender" "Xfixes" "X11" "pthread" "xcb" "Xau" "Xdmcp"
    )
else()
    message(FATAL_ERROR "Invalid operating system")
endif()


target_include_directories(myvk
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${VULKAN_SDK_PATH}/Include"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/fw"
        "${GLFW_PATH}/include"
        "${GLM_PATH}"
        "${ASSIMP_PATH}/include"
        "${STB_PATH}"
)

target_compile_features(myvk 
    PUBLIC 
        cxx_std_17
)

if(MSVC)
  target_compile_options(myvk PRIVATE /W3 /WX /MP)
else()
  target_compile_options(myvk PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

include(GNUInstallDirs)

install(TARGETS myvk EXPORT myvk-config
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}) # This is for Windows

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

export(TARGETS myvk FILE myvkConfig.cmake)
