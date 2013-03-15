
C_FLAGS= -Wall -g -ggdb -O0  -D__DEBUG

all:
	g++ $(C_FLAGS) -o thread_test ./main.cpp ../thread.cpp -lpthread -lrt

clean:	
	rm -f $(OBJECT) thread_test
