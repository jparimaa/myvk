include ../common.mk

EXECUTABLE = Example
BUILD_DIR = build/
OUTPUT = $(BUILD_DIR)$(EXECUTABLE)
SOURCES = \
	main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: framework $(OUTPUT) 

framework:
	make -C $(FRAMEWORK_PATH)

$(OUTPUT): $(OBJECTS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $(OUTPUT) $(LIBS)

%.o: %.cpp 
	$(CC) -c $(CFLAGS) $(INCLUDES) $<

run: $(OUTPUT)
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/explicit_layer.d $(OUTPUT)

.PHONY: clean

clean:
	rm -f *.o *.d *.spv $(OUTPUT) core buildlog

-include *.d
