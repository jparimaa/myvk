include ../variables.mk

OUTPUT = $(FRAMEWORK_LIB)
SOURCES = \
	Framework.cpp \
	API.cpp \
	Common.cpp \
	Context.cpp \
	Instance.cpp \
	Window.cpp \
	Device.cpp \
	SwapChain.cpp \
	RenderPass.cpp \
	Pipeline.cpp \
	Command.cpp \
	Image.cpp \
	Buffer.cpp \
	Texture.cpp \
	Sampler.cpp \
	Transformation.cpp \
	Time.cpp \
	Camera.cpp \
	Model.cpp \
	Mesh.cpp \
	Input.cpp \
	GUI.cpp \
	CameraController.cpp \
	imgui/imgui.cpp \
	imgui/imgui_draw.cpp \
	imgui/imgui_demo.cpp \
	imgui/imgui_impl_glfw_vulkan.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS) 
	mkdir -p $(FRAMEWORK_LIB_PATH)
	ar rcs $(OUTPUT) $(OBJECTS)

include ../common.mk

-include *.d
