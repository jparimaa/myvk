include ../variables.mk

EXECUTABLE = Subpass
BUILD_DIR = build/

OUTPUT = $(BUILD_DIR)$(EXECUTABLE)
SOURCES = \
	main.cpp \
	SubpassApp.cpp
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

$(OUTPUT): $(FRAMEWORK_LIB) $(VERT_SPIRS) $(FRAG_SPIRS) $(OBJECTS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $(OUTPUT) $(LIBS)

include ../common.mk

-include *.d
