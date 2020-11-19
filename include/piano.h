#ifndef PIANO_H
#define PIANO_H

#include "gl.h"

// TODO: write API for the following sections
void piano_init(unsigned int nrows, unsigned int ncols);
void draw_piano(void);
void draw_middle_key(int x, color_t color, enum state key);
void draw_sharp(int x, color_t color);
#endif