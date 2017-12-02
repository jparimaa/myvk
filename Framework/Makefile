include ../common.mk

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

%.o: %.cpp 
	$(CC) -c $(CFLAGS) $(INCLUDES) $<

.PHONY: clean

clean:
	rm -f *.o *.d $(OUTPUT) core buildlog

-include *.d
