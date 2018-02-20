framework:
	$(MAKE) -C $(FRAMEWORK_PATH)

%.o: %.cpp 
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

%_vert.spv: $(SHADER_DIR)%.vert
	$(GLSLANG_VALIDATOR) -V $< -o $@

%_frag.spv: $(SHADER_DIR)%.frag
	$(GLSLANG_VALIDATOR) -V $< -o $@

.PHONY: clean

clean:
	rm -f *.o imgui/*.o *.d imgui/*.d *.spv $(OUTPUT) core out *.txt vgcore* *~

run:
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib:$(ASSIMP_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/explicit_layer.d $(OUTPUT)
