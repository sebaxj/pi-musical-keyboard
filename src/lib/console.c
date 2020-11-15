#include "console.h"
#include <stdarg.h>
#include "gl.h"
#include "malloc.h"
#include "timer.h"
#include "printf.h"

#define _WIDTH 640
#define _HEIGHT 512
#define MAX_BUFFER 160
#define _NROWS 10
#define _NCOLS 20

static void process_char(char ch);

struct cursor {
    unsigned int pos_col;
    unsigned int pos_row;
};

static struct cursor cursor_position;
static int (*ptr)[_NROWS][_NCOLS];

void console_init(unsigned int nrows, unsigned int ncols)
{
    // initialize the cursor position in the top left
    cursor_position.pos_col = 0;
    cursor_position.pos_row = 0;

    ptr = malloc(nrows * ncols * sizeof(int));

    // initialize graphics library
    gl_init(_WIDTH, _HEIGHT, GL_SINGLEBUFFER);
    // initialize the 2D array, disp, of size nrows and ncols
    for(int y = 0; y < gl_get_height(); y += gl_get_char_height()) {
        for(int x = 0; x < gl_get_width(); x += gl_get_char_width()) {
            *ptr[y][x] = ' ';
            gl_draw_char(x, y, *ptr[y][x], GL_BLACK);
        }
    }
}

void console_clear(void)
{
    for(int y = 0; y < gl_get_height(); y += gl_get_char_height()) {
        for(int x = 0; x < gl_get_width(); x += gl_get_char_width()) {
            *ptr[y][x] = ' ';
            gl_draw_char(x, y, *ptr[y][x], GL_BLACK);
        }
    }
    gl_clear(GL_BLACK);
    gl_swap_buffer();
    cursor_position.pos_col = 0;
    cursor_position.pos_row = 0;
}

int console_printf(const char *format, ...)
{
    char buffer[MAX_BUFFER];
    va_list args;
    va_start(args, format);
    int size = vsnprintf(buffer, MAX_BUFFER, format, args);
    va_end(args);
    int i;
    for(i = 0; buffer[i] != '\0'; i++) {
        process_char(buffer[i]);
    }
    gl_swap_buffer();
    return size;
}

static void process_char(char ch)
{
    switch(ch) {
        case '\b': // backspace
            cursor_position.pos_col -= gl_get_char_width();

            // store current char at cursor position
            int curr_char =  *ptr[cursor_position.pos_row][cursor_position.pos_col];
            *ptr[cursor_position.pos_row][cursor_position.pos_col] = curr_char;

            // draw stored character in black to make it "vanish"
            gl_draw_char(cursor_position.pos_col, cursor_position.pos_row, *ptr[cursor_position.pos_row][cursor_position.pos_col], GL_BLACK);
            break;
        case '\r': // carriage return
            cursor_position.pos_col = 0; 
            break;
        case '\n': // new line
            cursor_position.pos_row += gl_get_char_height();
            cursor_position.pos_col = 0;
            break;
        case '\f': // form feed
            console_clear();
            break;
        default: // ordinary char
            *ptr[cursor_position.pos_row][cursor_position.pos_col] = (int)ch;
            gl_draw_char(cursor_position.pos_col, cursor_position.pos_row, *ptr[cursor_position.pos_row][cursor_position.pos_col], GL_AMBER);
            if(cursor_position.pos_col + (2 * gl_get_char_width()) > gl_get_width()) { // horizontal wrapping
                cursor_position.pos_row += gl_get_char_height();
                cursor_position.pos_col = 0;
            } else if(cursor_position.pos_row + gl_get_char_height() > gl_get_height()) { // vertical scrolling
                console_clear(); 
            } else {
                cursor_position.pos_col += gl_get_char_width();
            }
            break;
    }
}