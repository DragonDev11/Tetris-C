#ifndef CLASSES_H
#define CLASSES_H

#include <stdint.h>

uint8_t T_pattern[4] = {
    0b0111,
    0b0010,
    0b0000,
    0b0000
};

uint8_t L_pattern[4] = {
    0b0001,
    0b0111,
    0b0000,
    0b0000
};

uint8_t J_pattern[4] = {
    0b0100,
    0b0111,
    0b0000,
    0b0000
};

uint8_t I_pattern[4] = {
    0b0001,
    0b0001,
    0b0001,
    0b0001
};

uint8_t Z_pattern[4] = {
    0b0011,
    0b0110,
    0b0000,
    0b0000
};

uint8_t reverse_Z_pattern[4] = {
    0b0110,
    0b0011,
    0b0000,
    0b0000
};

uint8_t square_pattern[4] = {
    0b0011,
    0b0011,
    0b0000,
    0b0000
};

typedef struct{
    int r,g,b;
}Color;

typedef struct{
    uint8_t pattern[4];
    Color color;
    int max_width;
    int max_height;
}Shape;

typedef struct Vertex Vertex;

struct Vertex{
    float x,y;
    Vertex* next;
};

typedef struct{
    Shape shape;
    int row,col;
}Face;

#endif