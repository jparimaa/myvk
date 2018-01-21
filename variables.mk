CC = g++
CFLAGS = -std=c++1z -MMD -MT $@ -Wall -Wextra -ggdb

VULKAN_SDK_PATH = ../../../Tools/VulkanSDK/1.0.61.1/x86_64
GLSLANG_VALIDATOR = $(VULKAN_SDK_PATH)/bin/glslangValidator
STB_INCLUDE_PATH = ../../../Tools/stb
FRAMEWORK_PATH = ../Framework
FRAMEWORK_LIB_PATH = $(FRAMEWORK_PATH)/lib
FRAMEWORK_LIB = $(FRAMEWORK_LIB_PATH)/framework.a
INCLUDES = -I$(VULKAN_SDK_PATH)/include -I$(STB_INCLUDE_PATH)
LIBS = $(FRAMEWORK_LIB) \
	-L$(VULKAN_SDK_PATH)/lib -lvulkan \
	`pkg-config --static --libs glfw3`
