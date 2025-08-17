#ifndef CLASSES_H
#define CLASSES_H

#include <stdint.h>

uint8_t T_pattern[4] = {
    0b1110,
    0b0100,
    0b0000,
    0b0000
};

uint8_t L_pattern[4] = {
    0b1000,
    0b1110,
    0b0000,
    0b0000
};

uint8_t J_pattern[4] = {
    0b0010,
    0b1110,
    0b0000,
    0b0000
};

uint8_t I_pattern[4] = {
    0b1000,
    0b1000,
    0b1000,
    0b1000
};

typedef struct{
    int r,g,b;
}Color;

typedef struct{
    uint8_t pattern[4];
    Color color;
}Shape;

typedef struct Vertex Vertex;

struct Vertex{
    float x,y;
    Vertex* next;
};

typedef struct{
    Vertex* vertices;
    int total_vertices;
    Shape shape;
}Face;

#endif