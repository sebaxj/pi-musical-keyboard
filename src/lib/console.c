#include "console.h"
#include <stdarg.h>
#include "gl.h"
#include "malloc.h"
#include "printf.h"
#include "strings.h"
#include "timer.h"

static void process_char(char ch);

static void draw_text(char* text); 

//to track cursor's location 
struct location { 
	unsigned int x; 
	unsigned int y; 
}; 

struct console { 
	struct location cursor; 
	char* text; 
	color_t background; 
	color_t text_color; 
	unsigned int n_rows; 
	unsigned int n_cols;
}; 

static struct console my_console; 

#define MAX_OUTPUT_LEN 1024 

void console_init(unsigned int nrows, unsigned int ncols)
{    
    // cursor is in the home position 
    my_console.cursor.x = 0; 
    my_console.cursor.y = 0; 

    my_console.n_rows = nrows; 
    my_console.n_cols = ncols; 

    my_console.background = GL_AMBER;
    my_console.text_color = GL_BLACK; 

    gl_init(ncols * gl_get_char_width(), nrows * gl_get_char_height(), GL_SINGLEBUFFER);

    gl_clear(my_console.background); 

    my_console.text = malloc(nrows*ncols); 
    memset(my_console.text, 0x0, nrows*ncols); // text start empty
    
}

void console_clear(void)
{
    // bring cursor back to home position 
    my_console.cursor.x = 0; 
    my_console.cursor.y = 0; 

    gl_clear(my_console.background);

    memset(my_console.text, 0x0, my_console.n_rows * my_console.n_cols); // clear console text  
    
}

int console_printf(const char *format, ...)
{
    char buf[MAX_OUTPUT_LEN];
    va_list ap; 
    va_start(ap, format);
    int result = vsnprintf(buf, MAX_OUTPUT_LEN, format, ap); 
    va_end(ap); 

    int count = 0; 
    while(buf[count] != '\0'){ 
	   process_char(buf[count]); 
 	   count++; 
    }  
    //printf("%s\n", my_console.text);  
    return result;
}

static void process_char(char ch) // horizontal wrapping taken into account 
{
    // if ordinary char: inserts ch into contents at current position
    // of cursor, cursor advances one position
    // if special char: (\r \n \f \b) handle according to specific function
    if (ch == '\b') {
	    if (my_console.cursor.x == 0) { // handling going back to previous line 
		    my_console.cursor.y -= gl_get_char_height(); 
		    my_console.cursor.x = (my_console.n_cols - 1) * gl_get_char_width(); 
	    } else { 
	    	    my_console.cursor.x -= gl_get_char_width(); 
	    }  
	    gl_draw_rect(my_console.cursor.x, my_console.cursor.y, gl_get_char_width(), gl_get_char_height(), my_console.background); 
    } else if (ch == '\r') { 
	    my_console.cursor.x = 0; 
    }else if (ch == '\n') {
	    my_console.cursor.x = 0; 
	    my_console.cursor.y += gl_get_char_height(); 
    } else if (ch == '\f') console_clear();  
    else { 
	    if ( (my_console.cursor.x >=  my_console.n_cols*gl_get_char_width()) && (my_console.cursor.y < ((my_console.n_rows - 1) *gl_get_char_height()))) { // horizontal wrapping
		my_console.cursor.x = 0; 
		my_console.cursor.y += gl_get_char_height();  
	    // vertical wrapping 		
	    } else if (my_console.cursor.y >= my_console.n_rows * gl_get_char_height()){ // return in the middle of a lign
		    my_console.cursor.x = 0; 
		    memcpy(my_console.text, my_console.text + my_console.n_cols, my_console.n_cols * (my_console.n_rows -1));
		    memset(my_console.text + my_console.n_cols * (my_console.n_rows - 1), 0x0, my_console.n_cols); // set last row empty 
		    draw_text(my_console.text);
		    my_console.cursor.y -= gl_get_char_height(); 
	    } else if (my_console.cursor.y == (my_console.n_rows - 1) * gl_get_char_height() && my_console.cursor.x >= my_console.n_cols*gl_get_char_width()){
		    // vertical wrapping when combined with horizontal wrapping 
		    my_console.cursor.x = 0; 
		    memcpy(my_console.text, my_console.text + my_console.n_cols, my_console.n_cols * (my_console.n_rows -1)); 
		    memset(my_console.text + my_console.n_cols * (my_console.n_rows - 1), 0x0, my_console.n_cols); // set last row empty  
		    draw_text(my_console.text);
	    }
	    gl_draw_rect(my_console.cursor.x, my_console.cursor.y, gl_get_char_width(), gl_get_char_height(), my_console.background); 
	    gl_draw_char(my_console.cursor.x, my_console.cursor.y, ch, my_console.text_color); 
	    my_console.text[(my_console.cursor.x)/gl_get_char_width() + my_console.n_cols * (my_console.cursor.y)/gl_get_char_height()] = ch; 
	    my_console.cursor.x += gl_get_char_width();           
    } 
}

static void draw_text(char * text){ 
	// clear screen without clearing the text
	gl_draw_rect(0, 0, my_console.n_cols*gl_get_char_width(), my_console.n_rows*gl_get_char_height(), my_console.background); 

	// draw first n-1 cols of text into screen 
	char (* text_2d)[my_console.n_cols] = (char (*)[my_console.n_cols])text; 
	for (int i = 0; i< my_console.n_cols; i++){ 
	    for (int j = 0; j < my_console.n_rows; j++) { 
	        gl_draw_char(i*gl_get_char_width(), j*gl_get_char_height(), text_2d[j][i], my_console.text_color);
	    } 
	}	
} 

//enum state { MIDDLE_KEY, RIGHT_KEY, LEFT_KEY};

/*
enum piano_keys_x { 
	key_A,
	key_B,
	key_C, 
	key_D, 
	key_E, 
	key_F, 
	key_G, 
	key_last_A, 
}; 
*/



//static int piano_keys[8];

static int width_console = 0;
static int height_console = 0;
static int piano_width = 0;
static int piano_start_y = 0;
static int piano_start_x = 0;
static int piano_height = 0;
static int white_key_width = 0;
static int black_key_height = 0;
static int black_key_width = 0;

void draw_middle_key(int x, color_t color, enum state key); 
void draw_sharp(int x, color_t color); 
/*

void initialize(void){
	width_console = my_console.n_cols*gl_get_char_width(); 
	height_console = my_console.n_rows*gl_get_char_height(); 
	piano_width = width_console*9/10;
	piano_start_y = gl_get_char_height() + height_console * 5/100; 
	piano_start_x = width_console*5/100; 
	piano_height = (my_console.n_rows - 3) * gl_get_char_height(); 
	white_key_width = piano_width/8; 
	black_key_height = 3*piano_height/5;
	black_key_width = 2*white_key_width/3;
} 
*/

void draw_piano(void){
	console_clear(); 
	console_printf("Welcome to the piano keyboard. Have fun!\n"); 

	
	width_console = my_console.n_cols*gl_get_char_width(); 
	height_console = my_console.n_rows*gl_get_char_height(); 
	piano_width = width_console*9/10;
	piano_start_y = gl_get_char_height() + height_console * 5/100; 
	piano_start_x = width_console*5/100; 
	piano_height = (my_console.n_rows - 3) * gl_get_char_height(); 
	white_key_width = piano_width/8; 
	black_key_height = 3*piano_height/5;
	black_key_width = 2*white_key_width/3;
	

	for (int i = 0; i < 8; i++){
		piano_keys[i] = piano_start_x + i * white_key_width; 
	} 

	// draw white keys 
	gl_draw_rect(piano_start_x, piano_start_y, piano_width, piano_height, GL_WHITE); 
        for (int i = 0; i < 9; i++) { 	
		gl_draw_vertical_line(piano_start_x + i*white_key_width, piano_start_y, piano_height, GL_BLACK);
		
	} 
	// draw black keys 
	// draw left black key - only for esthetics 
	gl_draw_rect(piano_start_x, piano_start_y, black_key_width/2, black_key_height, GL_BLACK); 
	// A sharp 	
	gl_draw_rect(piano_start_x+ 2*white_key_width/3, piano_start_y, black_key_width, black_key_height, GL_BLACK); 
	// C sharp and D sharp 
	for (int i = 2; i < 4; i++) {
		gl_draw_rect(piano_start_x + i*white_key_width + 2*white_key_width/3, piano_start_y, black_key_width, black_key_height, GL_BLACK); 
	} 	
	// F sharp and G sharp 
	for (int i = 5; i < 7; i++) { 
		gl_draw_rect(piano_start_x + i*white_key_width + 2*white_key_width/3, piano_start_y, black_key_width, black_key_height, GL_BLACK); 
	} 
	// draw last black key - for esthetics 
	gl_draw_rect(piano_start_x + 7*white_key_width + 2*white_key_width/3, piano_start_y, black_key_width/2, black_key_height, GL_BLACK); 
}

void draw_middle_key(int x, color_t color, enum state key){ // surrounded by sharps 
	// figure out what type of note it is 
	if (key == 0) { // sharp to the left and to the right 
		gl_draw_rect(x + white_key_width/3, piano_start_y, white_key_width/3, black_key_height, color); 
	} else if (key == 1) { // sharp to the left
		gl_draw_rect(x, piano_start_y, 2*white_key_width/3, black_key_height, color); 
	} else { // sharp to the right
		gl_draw_rect(x+white_key_width/3, piano_start_y, 2*white_key_width/3, black_key_height, color); 
	} 
	gl_draw_rect(x, piano_start_y + black_key_height, white_key_width, piano_height - black_key_height, color);

	// draw the borders of the note 
        gl_draw_vertical_line(x, piano_start_y, piano_height, GL_BLACK); 	
	gl_draw_vertical_line(x + white_key_width, piano_start_y, piano_height, GL_BLACK); 
} 

void draw_sharp(int x, color_t color) // x is the start x coordinate of the white corresponding key (A for A_sharp)
{ 
	gl_draw_rect(x + 2*white_key_width/3, piano_start_y, black_key_width, black_key_height, color); 
} 	


