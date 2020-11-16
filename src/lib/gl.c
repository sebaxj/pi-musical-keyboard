#include "gl.h"
#include "fb.h"
#include "font.h"
#include "printf.h"
#include "fb_ref.h"

static gl_mode_t mode_used; 

void gl_init(unsigned int width, unsigned int height, gl_mode_t mode)
{
    fb_init(width, height, 4, mode);    // use 32-bit depth always for graphics library
    mode_used = mode; 
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
    color_t color = 0; 
    color |= 0xff << 24; 
    color |= r << 16; 
    color |= g << 8; 
    color |= b; 
    return color; 
}

void gl_clear(color_t c)
{
    // clear 2_d 
       	unsigned int row_len = fb_get_pitch()/sizeof(unsigned int); 
	unsigned int (*im)[row_len] = fb_get_draw_buffer();  
	for (int y=0; y < fb_get_height(); y++){ 
       	 	for (int x=0; x < row_len; x++){
            		im[y][x] = c; 
        	} 
    	} 
} 
   
	
void gl_draw_pixel(int x, int y, color_t c)
{
    unsigned int row_len = fb_get_pitch()/sizeof(unsigned int); 
    unsigned int (*im)[row_len] = fb_get_draw_buffer(); 
    if ((x < fb_get_width()) && (x >= 0) && (y >= 0) && (y < fb_get_height())) { 
    	    im[y][x] = c; 
	    //for (int i = 0; i < row_len; i++){
	//	    im[y][i] = c; 
	    //} 
    } 
}

color_t gl_read_pixel(int x, int y)
{
    unsigned int row_len = fb_get_pitch()/sizeof(unsigned int); 
    unsigned int (*im)[row_len] = fb_get_draw_buffer(); 
    if (x < fb_get_width()&& x >= 0 && y >= 0 && y < fb_get_height()) { 
        return im[y][x];
    } 
    return 0; 
}

void gl_draw_rect(int x, int y, int w, int h, color_t c)
{
    int w_rec = (w < fb_get_width()-x) ? w : (fb_get_width()-x); 
    int h_rec = (h < fb_get_height() - y) ? h : (fb_get_height()-y); 
    for (int i = 0; i < w_rec; i++) { 
        for (int j = 0; j < h_rec; j++) { 
	    gl_draw_pixel(x+i, y+j, c); 
        } 
    } 
}

void gl_draw_char(int x, int y, int ch, color_t c)
{
    size_t buflen = font_get_size(); 
    unsigned char buf[buflen]; 

    int proceed = font_get_char(ch, buf, buflen); 

    if (proceed){ 
	    unsigned int w_char = gl_get_char_width(); 
	    unsigned int h_char = gl_get_char_height(); 
	    for (int i = 0; i < w_char; i++){
		for (int j = 0; j < h_char; j++){
		    if (buf[i+w_char*j]) {
			    gl_draw_pixel(x+i, y+j, c); //only draw pixels within the bounds 
 	            } 
                } 
            }
    }
} 

void gl_draw_string(int x, int y, const char* str, color_t c)
{
    int count = 0; 
    unsigned int w_char = gl_get_char_width(); 
    while (str[count] != '\0') { 
    	gl_draw_char(x+w_char*count, y, str[count], c); 
    	count++; 
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
