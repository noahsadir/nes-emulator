CFLAGS = -Wall -pedantic-errors
SRC = ./src
OBJ = ./obj
BIN = ./bin

default: clean compile link

compile:
	mkdir -p $(OBJ)
	gcc $(CFLAGS) -g -O -c $(SRC)/*.c
	mv *.o $(OBJ)/

link:
	mkdir -p $(BIN)
	gcc -o $(BIN)/emulator $(OBJ)/*.o -lSDL2

clean:
	rm -f $(OBJ)/*
	rm -f $(BIN)/*
