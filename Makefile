
INCLUDES= -I ./include
FLAGS= -g

OBJECTS=./build/memory.o ./build/registers.o

all: ${OBJECTS}
	gcc $(FLAGS) $(INCLUDES) ./src/main.c ${OBJECTS} -lSDL2 -o ./bin/main

./build/memory.o:src/memory.c
	gcc $(FLAGS) $(INCLUDES) ./src/memory.c -c -o ./build/memory.o

./build/registers.o:src/registers.c
	gcc $(FLAGS) $(INCLUDES) ./src/registers.c -c -o ./build/registers.o

clean:
	rm -rf build/*
