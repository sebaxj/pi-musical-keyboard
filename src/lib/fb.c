#include "fb.h"
#include "mailbox.h"
#include "printf.h"
#include "assert.h"

#define debug_mode 0

typedef struct {
    unsigned int width;       // width of the physical screen
    unsigned int height;      // height of the physical screen
    unsigned int virtual_width;  // width of the virtual framebuffer
    unsigned int virtual_height; // height of the virtual framebuffer
    unsigned int pitch;       // number of bytes per row
    unsigned int bit_depth;   // number of bits per pixel
    unsigned int x_offset;    // x of the upper left corner of the virtual fb
    unsigned int y_offset;    // y of the upper left corner of the virtual fb
    unsigned int framebuffer; // pointer to the start of the framebuffer
    unsigned int total_bytes; // total number of bytes in the framebuffer
} fb_config_t;

// fb is volatile because the GPU will write to it
static volatile fb_config_t fb __attribute__ ((aligned(16)));

// static variable for fb.c to track mode of fb
static fb_mode_t fb_mode;

void fb_init(unsigned int width, unsigned int height, unsigned int depth_in_bytes, fb_mode_t mode)
{
    fb.width = width;
    fb.virtual_width = width;
    fb.height = height;
    if(mode == FB_DOUBLEBUFFER) {
        fb.virtual_height = height * 2; // twice physical height for space for 2 fb
    } else {
        fb.virtual_height = height;
    }
    fb_mode = mode;
    fb.bit_depth = depth_in_bytes * 8; // convert number of bytes to number of bits
    fb.x_offset = 0;
    fb.y_offset = 0; // front fb

    // the manual requires we to set these value to 0
    // the GPU will return new values
    fb.pitch = 0;
    fb.framebuffer = 0;
    fb.total_bytes = 0;

    // Send address of fb struct to the GPU
    mailbox_write(MAILBOX_FRAMEBUFFER, (unsigned int)&fb);
    // Read response from GPU
    assert(mailbox_read(MAILBOX_FRAMEBUFFER) == 0); // confirm successful config

    #if debug_mode == 1
    /* For debugging purposes only - delete before submission */
    printf("Framebuffer successfully configured.\n");
    printf("physical size = %d x %d\n", fb.width , fb.height);
    printf("virtual size = %d x %d\n", fb.virtual_width , fb.virtual_height);
    printf("depth = %d bits\n", fb.bit_depth );
    printf("pitch = %d bytes per row \n", fb.pitch );
    printf("total bytes = %d\n", fb.total_bytes );
    printf("buffer address for first buffer = %p\n", (void *)fb.framebuffer);
    if(fb.height != fb.virtual_height) {
        unsigned int double_buff_addr = fb.framebuffer + (fb.total_bytes / 2);
        printf("predicted buffer address for second buffer = %x\n", double_buff_addr);
        printf("buffer address for second buffer = %p\n", (unsigned char *)fb.framebuffer + (fb.total_bytes / 2));
    }
    #endif
    
    /*******************************************************************************/
}


void fb_swap_buffer(void)
{
    if(fb_mode == FB_DOUBLEBUFFER) {
        if(fb.y_offset != 0) {
            fb.y_offset = 0;
        } else {
            fb.y_offset = fb.height;
        }

        // Send address of fb struct to the GPU
        mailbox_write(MAILBOX_FRAMEBUFFER, (unsigned int)&fb);
        // Read response from GPU
        assert(mailbox_read(MAILBOX_FRAMEBUFFER) == 0); // confirm successful config
    }
}

void* fb_get_draw_buffer(void)
{
    if(fb.y_offset == 0 && fb_mode == FB_DOUBLEBUFFER) {
        return (void *)(fb.framebuffer + (fb.pitch * fb.height));
    } 
    return (void *)fb.framebuffer;
}

unsigned int fb_get_width(void)
{
    return fb.width;
}

unsigned int fb_get_height(void)
{
    return fb.height;
}

unsigned int fb_get_depth(void)
{
    return fb.bit_depth / 8;
}

unsigned int fb_get_pitch(void)
{
    return fb.pitch;
}