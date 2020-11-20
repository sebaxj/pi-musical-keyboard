#ifndef PIANO_H
#define PIANO_H

#include "gl.h"

struct piano_console {
    unsigned int width_console;
    unsigned int height_console;
    unsigned int piano_width;
    unsigned int piano_start_y;
    unsigned int piano_start_x;
    unsigned int piano_height;
    unsigned int white_key_width;
    unsigned int black_key_width;
    unsigned int black_key_height;
};

// Array to store instrument names.
const char *instruments[] = {"electric bass", "electric guitar", "organ", "vocals", "piano", "violin"};

// TODO: write API for the following sections
void piano_init(unsigned int nrows, unsigned int ncols);
void draw_piano(void);
void draw_middle_key(int x, color_t color, enum state key);
void draw_sharp(int x, color_t color);
#endif
