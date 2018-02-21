include ../variables.mk

EXECUTABLE = PBR
BUILD_DIR = build/

OUTPUT = $(BUILD_DIR)$(EXECUTABLE)
SOURCES = \
	main.cpp \
	PBRApp.cpp \
	EquirectangularHDR.cpp
OBJECTS = $(SOURCES:.cpp=.o)

SHADER_DIR = shaders/
VERTEX_SHADERS = \
	skybox.vert \
	equirectangular_hdr.vert
FRAGMENT_SHADERS = \
	skybox.frag \
	equirectangular_hdr.frag
FRAG_SPIRS = $(FRAGMENT_SHADERS:.frag=_frag.spv)
VERT_SPIRS = $(VERTEX_SHADERS:.vert=_vert.spv)

all: $(OUTPUT) 

$(OUTPUT): framework $(VERT_SPIRS) $(FRAG_SPIRS) $(OBJECTS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $(OUTPUT) $(LIBS)

include ../common.mk

-include *.d
