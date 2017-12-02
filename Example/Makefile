include ../variables.mk

EXECUTABLE = Example
BUILD_DIR = build/
OUTPUT = $(BUILD_DIR)$(EXECUTABLE)
SOURCES = \
	main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: framework $(OUTPUT) 

$(OUTPUT): $(OBJECTS) $(FRAMEWORK_LIB)
	mkdir -p $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $(OUTPUT) $(LIBS)

include ../common.mk

-include *.d
