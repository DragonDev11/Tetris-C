all:
	gcc -Isrc/include -Lsrc/lib -o main main.c array.c -lmingw32 -lSDL3