#include "piano.h"
#include "console.h"
#include "gl.h"

static struct piano_console piano;
void draw_middle_key(int x, color_t color, enum state key); 
void draw_sharp(int x, color_t color); 

void piano_init(unsigned int nrows, unsigned int ncols) {
    piano.width_console = ncols * gl_get_char_width();
    piano.height_console = nrows * gl_get_char_height();

    // initialize piano graphic
	piano.piano_width = piano.width_console*9/10;
	piano.piano_start_y = 3 * gl_get_char_height() + piano.height_console * 5/100; 
	piano.piano_start_x = piano.width_console*5/100; 
	piano.piano_height = (nrows - 5) * gl_get_char_height(); 
	piano.white_key_width = piano.piano_width/8; 
	piano.black_key_height = 3*piano.piano_height/5;
	piano.black_key_width = 2*piano.white_key_width/3;
	
	for (int i = 0; i < 8; i++){
		piano_keys[i] = piano.piano_start_x + i * piano.white_key_width; 
	} 
}

void draw_piano(void) {
	console_clear(); 
	console_printf("Use Keys 'tab' through ']' to play the piano.\n");
    	console_printf("Press ESC to return to console, or 'Z' to choose instrument\n"); 
    
	// draw white keys 
	gl_draw_rect(piano.piano_start_x, piano.piano_start_y, piano.piano_width, piano.piano_height, GL_WHITE); 
        for (int i = 0; i < 9; i++) { 	
		gl_draw_vertical_line(piano.piano_start_x + i*piano.white_key_width, piano.piano_start_y, piano.piano_height, GL_BLACK);
		
	} 
	// draw black keys 
	// draw left black key - only for aesthetics 
	gl_draw_rect(piano.piano_start_x, piano.piano_start_y, piano.black_key_width/2, piano.black_key_height, GL_BLACK); 
	// A sharp 	
	gl_draw_rect(piano.piano_start_x+ 2*piano.white_key_width/3, piano.piano_start_y, piano.black_key_width, piano.black_key_height, GL_BLACK); 
	// C sharp and D sharp 
	for (int i = 2; i < 4; i++) {
		gl_draw_rect(piano.piano_start_x + i*piano.white_key_width + 2*piano.white_key_width/3, piano.piano_start_y, piano.black_key_width, piano.black_key_height, GL_BLACK); 
	} 	
	// F sharp and G sharp 
	for (int i = 5; i < 7; i++) { 
		gl_draw_rect(piano.piano_start_x + i*piano.white_key_width + 2*piano.white_key_width/3, piano.piano_start_y, piano.black_key_width, piano.black_key_height, GL_BLACK); 
	} 
	// draw last black key - for aesthetics 
	gl_draw_rect(piano.piano_start_x + 7*piano.white_key_width + 2*piano.white_key_width/3, piano.piano_start_y, piano.black_key_width/2, piano.black_key_height, GL_BLACK); 
}

void draw_middle_key(int x, color_t color, enum state key){ // surrounded by sharps 
	// figure out what type of note it is 
	if (key == 0) { // sharp to the left and to the right 
		gl_draw_rect(x + piano.white_key_width/3, piano.piano_start_y, piano.white_key_width/3, piano.black_key_height, color); 
	} else if (key == 1) { // sharp to the left
		gl_draw_rect(x, piano.piano_start_y, 2*piano.white_key_width/3, piano.black_key_height, color); 
	} else { // sharp to the right
		gl_draw_rect(x+piano.white_key_width/3, piano.piano_start_y, 2*piano.white_key_width/3, piano.black_key_height, color); 
	} 
	gl_draw_rect(x, piano.piano_start_y + piano.black_key_height, piano.white_key_width, piano.piano_height - piano.black_key_height, color);

	// draw the borders of the note 
        gl_draw_vertical_line(x, piano.piano_start_y, piano.piano_height, GL_BLACK); 	
	gl_draw_vertical_line(x + piano.white_key_width, piano.piano_start_y, piano.piano_height, GL_BLACK); 
} 

void draw_sharp(int x, color_t color) // x is the start x coordinate of the white corresponding key (A for A_sharp)
{ 
	gl_draw_rect(x + 2*piano.white_key_width/3, piano.piano_start_y, piano.black_key_width, piano.black_key_height, color); 
} 
