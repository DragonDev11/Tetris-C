all:
	gcc -Isrc/include -Lsrc/lib -o Tetris main.c array.c icon.o -lmingw32 -lSDL3