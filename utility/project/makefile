
C_FLAGS= -Wall -g -ggdb -O0  -D__DEBUG
C_FLAGS2=
SOURCES_FILES_CPP=$(wildcard ../*.cpp)
OBJECTS=$(patsubst %.cpp, obj/%.o, $(notdir $(SOURCES_FILES_CPP)))
TARGET=libutility.a

all: $(OBJECTS)
	g++ $(C_FLAGS) -o utility ./main.cpp $(OBJECTS) -lpthread -lrt

./obj/%.o:../%.cpp
	$(CXX)   $(C_FLAGS) -c $< -o $@
$(TARGET): $(OBJECTS)
	ar -r $(TARGET)  $(OBJECTS) $(C_FLAGS2)
	


clean:	
	rm -f $(OBJECT) utility main.o


