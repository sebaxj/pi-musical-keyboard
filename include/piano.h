#ifndef PIANO_H
#define PIANO_H

#include "gl.h"

/* 
 * Authors: Sebastian James, Isma Lemhadri 
 * Date: 20/11/2020
 */

/* 
 * Describes the state of a white key: 
 * - MIDDLE_KEY for keys surrounded by sharps
 * - RIGHT_KEY for keys with only a sharp to the right 
 * - LEFT_KEY for keys with only a sharp to the left 
 */
enum state { MIDDLE_KEY, RIGHT_KEY, LEFT_KEY}; 

int piano_keys[8]; 

enum piano_keys_x{
	key_A, 
	key_B, 
	key_C, 
	key_D, 
	key_E, 
	key_F, 
	key_G, 
	key_A_upper,
}; 


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

/* 
 *  Initializes the piano struct variables to adapt it to 
 *  the console size, to display a 1-octave piano keyboard. 
 * 
 */
void piano_init(unsigned int nrows, unsigned int ncols);

/* 
 *  Draws the piano keyboard, by first drawing the white keys, 
 *  then the black keys. 
 */
void draw_piano(void);

/* 
 *  Function made to color a given key, for the purpose of making 
 *  the keyboard interactive with the notes pressed by the user. 
 *  Depending on which white key it is (whether it's surrounded 
 *  by sharps, or has a sharp to its right, or a sharp to its left), 
 *  it colors the appropriate surface. 
 *  The x coordinate is the coordinate of the rectangle of 
 *  white_key_width and piano_heihgt associated to the white key. 
 */
void draw_middle_key(int x, color_t color, enum state key);

/* 
 * Function made to color a given black key. The x coordinate is the 
 * x start coordinate of the white key associated with that sharp. 
 */
void draw_sharp(int x, color_t color);

#endif
