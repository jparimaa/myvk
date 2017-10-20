CC = g++
CFLAGS = -std=c++11 -Wall -MMD -ggdb
VULKAN_SDK_PATH = ../../Tools/VulkanSDK/1.0.61.1/x86_64
INCLUDES = -I$(VULKAN_SDK_PATH)/include
LIBS = -L$(VULKAN_SDK_PATH)/lib -lvulkan `pkg-config --static --libs glfw3`
EXECUTABLE = VulkanTest
BUILD = build/
OUTPUT = $(BUILD)$(EXECUTABLE)
SOURCES = \
	main.cpp \
	TestApp.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(OBJECTS) $(OUTPUT)

$(OUTPUT): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $(OUTPUT) $(LIBS)

%.o: %.cpp 
	$(CC) -c $(CFLAGS) $(INCLUDES) $<

run: $(OUTPUT)
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/explicit_layer.d $(OUTPUT)

.PHONY: clean

clean:
	rm -f *.o *.d $(OUTPUT) core

-include *.d
