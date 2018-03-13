include ../variables.mk

EXECUTABLE = Example
BUILD_DIR = build/

OUTPUT = $(BUILD_DIR)$(EXECUTABLE)
SOURCES = \
	main.cpp \
	ExampleApp.cpp
OBJECTS = $(SOURCES:.cpp=.o)

SHADER_DIR = shaders/
VERTEX_SHADERS = \
	shader.vert
FRAGMENT_SHADERS = \
	shader.frag
FRAG_SPIRS = $(FRAGMENT_SHADERS:.frag=_frag.spv)
VERT_SPIRS = $(VERTEX_SHADERS:.vert=_vert.spv)

all:
	+$(MAKE) -C $(FRAMEWORK_PATH)
	+$(MAKE) $(OUTPUT)

$(OUTPUT): $(VERT_SPIRS) $(FRAG_SPIRS) $(OBJECTS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $(OUTPUT) $(LIBS)

include ../common.mk

-include *.d
