SOURCES=$(wildcard threads/*/*.cpp threads/*.cpp)
HEADERS=$(SOURCES:.cpp=.hpp)
FLAGS=-DDEBUG -g -Wall

all: main

main: $(SOURCES) $(HEADERS)
	mpic++ $(SOURCES) $(FLAGS) -o main

clear: clean

clean:
	rm main a.out

run: main
	mpirun --oversubscribe -np 5 -mca orte_base_help_aggregate 0 ./main
