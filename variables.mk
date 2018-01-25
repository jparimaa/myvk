CC = g++
CFLAGS = -std=c++1z -MMD -MT $@ -Wall -Wextra -ggdb

VULKAN_SDK_PATH = ../../../Tools/VulkanSDK/1.0.61.1/x86_64
GLSLANG_VALIDATOR = $(VULKAN_SDK_PATH)/bin/glslangValidator
STB_PATH = ../../../Tools/stb
ASSIMP_PATH = ../../../Tools/assimp-3.3.1

FRAMEWORK_PATH = ../Framework
FRAMEWORK_LIB_PATH = $(FRAMEWORK_PATH)/lib
FRAMEWORK_LIB = $(FRAMEWORK_LIB_PATH)/framework.a

INCLUDES = -I$(VULKAN_SDK_PATH)/include \
		   -I$(STB_PATH) \
		   -I$(ASSIMP_PATH)/include

LIBS = $(FRAMEWORK_LIB) \
	-L$(VULKAN_SDK_PATH)/lib -lvulkan \
	-L$(ASSIMP_PATH)/lib -lassimp \
	`pkg-config --static --libs glfw3`
