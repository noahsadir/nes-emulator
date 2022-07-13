CFLAGS = -Wall
SRC = ./src
OBJ = ./obj
BIN = ./bin

default: clean compile link

compile:
	mkdir -p $(OBJ)
	gcc $(CFLAGS) -g -O -c $(SRC)/exceptions.c -o $(OBJ)/exceptions.o
	gcc $(CFLAGS) -g -O -c $(SRC)/joypad.c -o $(OBJ)/joypad.o
	gcc $(CFLAGS) -g -O -c $(SRC)/ppu.c -o $(OBJ)/ppu.o
	gcc $(CFLAGS) -g -O -c $(SRC)/bus.c -o $(OBJ)/bus.o
	gcc $(CFLAGS) -g -O -c $(SRC)/cpu.c -o $(OBJ)/cpu.o
	gcc -g -O -c $(SRC)/main.c -o $(OBJ)/main.o

link:
	mkdir -p $(BIN)
	gcc -o $(BIN)/emulator $(OBJ)/joypad.o $(OBJ)/exceptions.o $(OBJ)/ppu.o $(OBJ)/bus.o $(OBJ)/cpu.o $(OBJ)/main.o -lSDL2

clean:
	rm -f $(OBJ)/*
	rm -f $(BIN)/*
