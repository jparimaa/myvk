CC = g++
CFLAGS = -std=c++1z -MMD -Wall -ggdb

VULKAN_SDK_PATH = ../../../Tools/VulkanSDK/1.0.61.1/x86_64
STB_INCLUDE_PATH = ../../../Tools/stb
FRAMEWORK_PATH = ../Framework
FRAMEWORK_LIB_PATH = $(FRAMEWORK_PATH)/lib
FRAMEWORK_LIB = $(FRAMEWORK_LIB_PATH)/framework.a
INCLUDES = -I$(VULKAN_SDK_PATH)/include -I$(STB_INCLUDE_PATH)
LIBS = -L$(VULKAN_SDK_PATH)/lib -lvulkan \
	`pkg-config --static --libs glfw3` \
	$(FRAMEWORK_LIB)
