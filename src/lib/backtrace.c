#include "backtrace.h"
#include "printf.h"

/*
 * Definitions and Constants
 */

#define BYTE_OFFSET_HEX 0xc
#define MSB_FF 0xff000000

const char *name_of(uintptr_t fn_start_addr)
{
    unsigned int *start = (unsigned int *)(fn_start_addr - 4);

    if((*(start) & ~0xff) == MSB_FF) {
        int name_len = *(start) & ~MSB_FF;
        return (char *)(start - (name_len / 4));
    }
    return "???";
}

int backtrace (frame_t f[], int max_frames)
{
    void *start_fp;
    __asm__("mov %0, fp" : "=r" (start_fp)); // initialize current fp of "backtrace"
    int *cur_fp = (int *)start_fp;
    
    volatile int i;
    for(i = 0; i < max_frames && *cur_fp != '\0' && (int *)(*(cur_fp - 3)) != 0; i++) {
        int *next_fp = (int *)(*(cur_fp - 3)); // pointer to next frames fp
        f[i].name = name_of((*next_fp) - BYTE_OFFSET_HEX);
        f[i].resume_addr = *(cur_fp - 1);
        f[i].resume_offset = f[i].resume_addr - (*next_fp - BYTE_OFFSET_HEX);
        cur_fp = next_fp;
    }
    return i;
}

void print_frames (frame_t f[], int n)
{
    for (int i = 0; i < n; i++)
        printf("#%d 0x%x at %s+%d\n", i, f[i].resume_addr, f[i].name, f[i].resume_offset);
}

void print_backtrace (void)
{
    int max = 50;
    frame_t arr[max];

    int n = backtrace(arr, max);
    print_frames(arr+1, n-1);   // print frames starting at this function's caller
}
