CFLAGS = -Wall

default: clean compile link

compile:
	cd src
	gcc $(CFLAGS) -g -O -c ./src/exceptions.c -o ./obj/exceptions.o
	gcc $(CFLAGS) -g -O -c ./src/joypad.c -o ./obj/joypad.o
	gcc $(CFLAGS) -g -O -c ./src/ppu.c -o ./obj/ppu.o
	gcc $(CFLAGS) -g -O -c ./src/bus.c -o ./obj/bus.o
	gcc $(CFLAGS) -g -O -c ./src/cpu.c -o ./obj/cpu.o
	gcc -g -O -c ./src/main.c -o ./obj/main.o

link:
	gcc -o ./bin/emulator ./obj/joypad.o ./obj/exceptions.o ./obj/ppu.o ./obj/bus.o ./obj/cpu.o ./obj/main.o -lSDL2

clean:
	rm -f ./obj/*.o
	rm -f ./bin/emulator
