framework:
	make -C $(FRAMEWORK_PATH)

%.o: %.cpp 
	$(CC) -c $(CFLAGS) $(INCLUDES) $<

.PHONY: clean

clean:
	rm -f *.o *.d *.spv $(OUTPUT) core buildlog *.txt vgcore*

run: $(OUTPUT)
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/explicit_layer.d $(OUTPUT)
