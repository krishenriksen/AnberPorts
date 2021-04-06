CC = gcc
CCFLAGS = -W -Wall

BINARY = AnberPortsSDL
LIBRARIES = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lGLESv1_CM
SOURCES = "main.c"

all:
	$(CC) $(CCFLAGS) $(INCLUDES) $(SOURCES) -o $(BINARY) $(LIBRARIES)

clean:
	rm -f $(BINARY)
