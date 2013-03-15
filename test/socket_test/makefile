
#C_FLAGS= -Wall -g3 -ggdb  -D_DEBUG
C_FLAGS= -g -ggdb -Wall -O0 -I../../utility/ -I../../tools/thread4z/ -I../../tools/log4z/ -I../../network/ -I../../network/epoll/
C_FLAGS2=-lpthread -lrt 
CXX=g++

SOURCES_FILES_CPP= $(wildcard ../../network/epoll/*.cpp)
OBJECTS=$(patsubst %.cpp, obj/%.o, $(notdir $(SOURCES_FILES_CPP)))

./obj/%.o:../../network/epoll/%.cpp
	$(CXX)  $(C_FLAGS) -c $< -o $@

PUBLICS="../../utility/utility.cpp ../../tools/thread4z/thread.cpp ../../tools/log4z/log4z.cpp"

all:$(OBJECTS)
	$(CXX) -o server_test $(C_FLAGS) $(C_FLAGS2) socket_server.cpp ../../utility/utility.cpp ../../tools/thread4z/thread.cpp ../../tools/log4z/log4z.cpp $(OBJECTS)

clean:	
	rm -fr $(OBJECTS) server_test ./log/*


