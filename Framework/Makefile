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
	Image.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS) 
	mkdir -p $(FRAMEWORK_LIB_PATH)
	ar rcs $(OUTPUT) $(OBJECTS)

include ../common.mk

-include *.d
