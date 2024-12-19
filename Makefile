CC = clang
IFLAGS = -I./lib/
LFLAGS = `pkg-config sdl3 --cflags --libs` -lsdl3_image -lm #<--- REVIEW
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 
SOURCES = ./src/*.c
TARGET = game # <---- CHANGE


all: default

default: 
	$(CC) $(IFLAGS) $(LFLAGS) $(CFLAGS) $(SOURCES) -o $(TARGET)

run: default
	./$(TARGET)

debug:
	$(CC) $(IFLAGS) $(LFLAGS) $(CFLAGS) $(SOURCES) -g -o $(TARGET)

clean:
	rm -f $(TARGET)