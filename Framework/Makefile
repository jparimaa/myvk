include ../variables.mk

OUTPUT = $(FRAMEWORK_LIB)
SOURCES = \
	Framework.cpp \
	Window.cpp \
	Context.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS) 
	mkdir -p $(FRAMEWORK_LIB_PATH)
	ar rcs $(OUTPUT) $(OBJECTS)

include ../common.mk

-include *.d
