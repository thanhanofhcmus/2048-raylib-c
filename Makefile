build: main.c
	cc -c main.c -o out/main.o -Wall -Wextra -Werror -pedantic $(shell pkg-config --cflags raylib)
	cc out/main.o -o out/main $(shell pkg-config --libs raylib)


run: build
	./out/main
