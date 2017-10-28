CC = g++
CFLAGS = -std=c++11 -MMD -Wall -ggdb
VULKAN_SDK_PATH = ../../Tools/VulkanSDK/1.0.61.1/x86_64
INCLUDES = -I$(VULKAN_SDK_PATH)/include
LIBS = -L$(VULKAN_SDK_PATH)/lib -lvulkan `pkg-config --static --libs glfw3`

EXECUTABLE = VulkanTest
BUILD_DIR = build/
OUTPUT = $(BUILD_DIR)$(EXECUTABLE)
SOURCES = \
	main.cpp \
	TestApp.cpp
OBJECTS = $(SOURCES:.cpp=.o)

GLSLANG_VALIDATOR = $(VULKAN_SDK_PATH)/bin/glslangValidator
SHADER_DIR = shaders/
VERTEX_SHADERS = \
	shader.vert
FRAGMENT_SHADERS = \
	shader.frag
FRAG_SHADER_BINARIES = $(FRAGMENT_SHADERS:.frag=_frag.spv)
VERT_SHADER_BINARIES = $(VERTEX_SHADERS:.vert=_vert.spv)

all: $(OUTPUT) $(VERT_SHADER_BINARIES) $(FRAG_SHADER_BINARIES)

$(OUTPUT): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $(OUTPUT) $(LIBS)

%.o: %.cpp 
	$(CC) -c $(CFLAGS) $(INCLUDES) $<

$(VERT_SHADER_BINARIES): %_vert.spv: $(SHADER_DIR)%.vert
	$(GLSLANG_VALIDATOR) -V $? -o $@

$(FRAG_SHADER_BINARIES): %_frag.spv: $(SHADER_DIR)%.frag
	$(GLSLANG_VALIDATOR) -V $? -o $@

run: $(OUTPUT)
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/explicit_layer.d $(OUTPUT)

.PHONY: clean

clean:
	rm -f *.o *.d *.spv $(OUTPUT) core

-include *.d
