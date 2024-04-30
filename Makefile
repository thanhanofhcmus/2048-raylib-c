.PHONY: build
build: main.o
	cc main.c -Wall -Wextra -Werror -pedantic $(shell pkg-config --libs --cflags raylib) -o main

run: build
	./main
