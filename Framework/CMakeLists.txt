cmake_minimum_required(VERSION 3.5)
project(myvk-fw VERSION 1.0.0 LANGUAGES CXX)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.12")
        message(FATAL_ERROR "Insufficient compiler version, version 19.12 in requires (VS 15 2017)")
    endif()
endif()

add_library(myvk
    inc/API.h
    inc/Buffer.h
    inc/Camera.h
    inc/CameraController.h
    inc/Command.h
    inc/Common.h
    inc/Context.h
    inc/Device.h
    inc/Framework.h
    inc/GUI.h
    inc/Image.h
    inc/Input.h
    inc/Instance.h
    inc/Mesh.h
    inc/Model.h
    inc/Pipeline.h
    inc/RenderPass.h
    inc/Sampler.h
    inc/SwapChain.h
    inc/Texture.h
    inc/Time.h
    inc/Transformation.h
    inc/Window.h
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
)

set(VULKAN_SDK_PATH $ENV{VULKAN_SDK} CACHE FILEPATH "Path to Vulkan SDK")

if(WIN32)
    set(VULKAN_LIB_PATH "${VULKAN_SDK_PATH}/Lib")
elseif(UNIX)
    message(FATAL_ERROR "Unix support is insufficient")
else()
    message(FATAL_ERROR "Invalid operating system")
endif()

target_link_libraries(myvk
    PUBLIC
        "${VULKAN_LIB_PATH}/vulkan-1.lib"
)

target_include_directories(myvk
    PUBLIC
        "${VULKAN_SDK_PATH}/Include"
        "${CMAKE_CURRENT_SOURCE_DIR}/inc"
)

target_compile_features(myvk 
    PUBLIC 
        cxx_std_17
)

include(GNUInstallDirs)

install(TARGETS myvk EXPORT myvk-config
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}) # This is for Windows

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

export(TARGETS myvk FILE myvkConfig.cmake)
