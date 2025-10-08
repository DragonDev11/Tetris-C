# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Isrc/include
LDFLAGS = -lm
SDL_LIBS = -Lsrc/lib -lSDL3

# Targets
TARGET = tetris
SRCS = main.c array.c
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(SDL_LIBS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(TARGET) $(OBJS)

# Run the game
run: $(TARGET)
	LD_LIBRARY_PATH=src/lib ./$(TARGET)

.PHONY: all clean run
