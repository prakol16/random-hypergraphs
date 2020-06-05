TARGET_CPPS := main.c
CPP_FILES := $(filter-out $(TARGET_CPPS),$(wildcard *.cpp))
OBJ_FILES := $(CPP_FILES:.cpp=.o)
H_FILES   := $(wildcard *.h)

CPP_FLAGS = --std=c++17 -Wall -Werror -Wpedantic -Ofast -march=native -Wno-unused-function

all: main

main: $(OBJ_FILES) main.o
	g++ -o $@ $^

%.o: %.cpp $(H_FILES) Makefile
	g++ -c $(CPP_FLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f *.o run-tests explore *~
