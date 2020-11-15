#include "gl.h"
#include "font.h"
#include "malloc.h"

void gl_init(unsigned int width, unsigned int height, gl_mode_t mode)
{
    fb_init(width, height, 4, mode);    // use 32-bit depth always for graphics library
}

void gl_swap_buffer(void)
{
    fb_swap_buffer();
}

unsigned int gl_get_width(void)
{
    return fb_get_width();
}

unsigned int gl_get_height(void)
{
    return fb_get_height();
}

color_t gl_color(unsigned char r, unsigned char g, unsigned char b)
{
    unsigned char color_arr[4] = {b, g, r, 0xff};
    int color = 0;
    for(int i = 0; i < 4; i++) {
        color = (color & ~(0xFF << (i << 3))) | (((int)color_arr[i]) << (i << 3));
    }

    return color;
}

void gl_clear(color_t c)
{
    for(int x = 0; x < fb_get_width(); x++) {
        for(int y = 0; y < fb_get_height(); y++) {
            gl_draw_pixel(x, y, c);
        }
    }
}

void gl_draw_pixel(int x, int y, color_t c)
{
    if(x < fb_get_width() && y < fb_get_height() && x >= 0 && y >= 0) {
        unsigned int per_row = fb_get_pitch() / fb_get_depth(); // length of row with padding
        unsigned int (*ptr)[per_row] = fb_get_draw_buffer();
        ptr[y][x] = c;
    }
}

color_t gl_read_pixel(int x, int y)
{
    if(x < fb_get_width() && y < fb_get_height() && x >= 0 && y >= 0) {
        unsigned int per_row = fb_get_pitch() / fb_get_depth(); // length of row with padding
        unsigned int (*ptr)[per_row] = fb_get_draw_buffer();
        return ptr[y][x];
    }
    return 0;
}

void gl_draw_rect(int x, int y, int w, int h, color_t c)
{
    for(int j = x; j < x + w; j++) {
        for(int i = y; i < y + h; i++) {
            gl_draw_pixel(j, i, c);
        }
    }
}

void gl_draw_char(int x, int y, int ch, color_t c)
{
    unsigned int size_of_buf = font_get_height() * font_get_width();
    unsigned char *buf = malloc(size_of_buf);
    int index = 0;
    if(font_get_char(ch, buf, size_of_buf)) {
        for(int i = y; i < y + font_get_height(); i++) { // buf stores pixels by column first, then row
            for(int j = x; j < x + font_get_width(); j++) {
                if(buf[index] == 0xff) {
                    gl_draw_pixel(j, i, c);
                }
                index++;
            }
        }
    }
    free(buf);
}

void gl_draw_string(int x, int y, const char* str, color_t c)
{
    while(*str != '\0') {
        gl_draw_char(x, y, (int)*str, c);
        x += gl_get_char_width();
        str++;
    }
}

unsigned int gl_get_char_height(void)
{
    return font_get_height();
}

unsigned int gl_get_char_width(void)
{
    return font_get_width();
}