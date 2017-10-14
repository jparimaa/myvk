VULKAN_SDK_PATH = ../../Tools/VulkanSDK/1.0.61.1/x86_64

CFLAGS = -std=c++11 -I$(VULKAN_SDK_PATH)/include
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan
BUILD = build/

VulkanTest: main.cpp
	g++ $(CFLAGS) -o $(BUILD)VulkanTest main.cpp $(LDFLAGS)

.PHONY: test clean

test: VulkanTest
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/explicit_layer.d  $(BUILD)VulkanTest

clean:
	rm -f $(BUILD)VulkanTest