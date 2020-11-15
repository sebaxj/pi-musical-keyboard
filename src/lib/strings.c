#include "strings.h"

#define HEX_BASE (int)16
#define min(x, y) (((x) < (y)) ? (x) : (y))

// Letter to ASCII decimal values
enum ASCII_NUM {
    ASCII_A = 65,
    ASCII_B = 66,
    ASCII_C = 67,
    ASCII_D = 68,
    ASCII_E = 69,
    ASCII_F = 70,
    ASCII_a = 97,
    ASCII_b = 98,
    ASCII_c = 99,
    ASCII_d = 100,
    ASCII_e = 101,
    ASCII_f = 102,
    ASCII_x = 120
    
};

// Decimal values
enum ASCII_DEC {
    DEC_0 = 0,
    DEC_9 = 9,
    DEC_A = 10,
    DEC_B = 11,
    DEC_C = 12,
    DEC_D = 13,
    DEC_E = 14,
    DEC_F = 15
};

// helper function to check bounds of ascii param
static int check_hex_param(char ch) {
    if(ch < '0' || (ch > '9' && ch < 'A') || (ch > 'F' && ch < 'a') || ch > 'f' || ch == '\0') {
        return 0;
    }
    return 1;
}

// helper function to raise a number to the power of
static int power(int base, int exp) {
    int result = 1;
    while(exp) {   
        result = result * base; 
        exp--; 
    }
    return result;
}

// helper function to convert a hex letter to the corresponding integer value
static int hextodec(int ascii) {
    if(ascii == ASCII_A || ascii == ASCII_a) {
        ascii = DEC_A; 
    } else if(ascii == ASCII_B || ascii == ASCII_b) {
        ascii = DEC_B;
    } else if(ascii == ASCII_C || ascii == ASCII_c) {
        ascii = DEC_C;
    } else if(ascii == ASCII_D || ascii == ASCII_d) {
        ascii = DEC_D;
    } else if(ascii == ASCII_E || ascii == ASCII_e) {
        ascii = DEC_E;
    } else if(ascii == ASCII_F || ascii == ASCII_f) {
        ascii = DEC_F;
    }
    return ascii;
}

void *memset(void *s, int c, size_t n)
{
    // cast void pointer to unsinged char pointer
    // convert int to unsigned char
    unsigned char *ptr = (unsigned char *)s;
    unsigned char c_char = (unsigned char)c;

    for(int i = 0; i < n; i++) {
        ptr[i] = c_char;
    }
    return ptr;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    // cast void ptr to char ptr
    const char *ch_src = (const char *)src;
    char *ch_dst = (char *)dst;

    for(int i = 0; i < n; i++) { // for bytes in src, set dst[i] to src[i]
        if(ch_src == ch_dst) {return NULL;} // if memory overlaps, return NULL or pointer NULL
        ch_dst[i] = ch_src[i];
    }

    return ch_dst;
}

size_t strlen(const char *s)
{
    int n = 0;
    while (s[n] != '\0') { // access Nth element in array char (pointer in mem)
        n++;
    }
    return n; // return number of characters 
}

int strcmp(const char *s1, const char *s2)
{
    int i = 0;
    for(; ((s1[i] != '\0' && s2[i] != '\0') && ((int)s1[i] == (int)s2[i])); i++) {
    }

    return ((int)s1[i] - (int)s2[i]);
}

size_t strlcat(char *dst, const char *src, size_t maxsize)
{
    int end_dst = strlen(dst);
    if(end_dst > maxsize) { // if not null terminated, return what would have been copied
        return maxsize + end_dst;
    }

    // for src is not null or maxsize - strlen(dst) - 1 (whichever comes first), copy char from src to dst
    for(int i = 0; i < maxsize - strlen(dst) - 1 && src[i] != '\0'; i++) {
        dst[end_dst] = src[i];
        end_dst++;
    }
    dst[end_dst] = '\0';

    return min(maxsize, (strlen(src) + (strlen(dst))));
}

unsigned int strtonum(const char *str, const char **endptr)
{
    unsigned int counter = 0;
    if(str[0] == ' ' || str[0] < '0' || str[0] > '9') return counter; // handle invalid request
    if(str[0] == '0' && str[1] == 'x') { // if second char is x
        // hex
        int n = 0; // hex place
        int length = strlen(str);
        for(int i = length; i > 1; i--) { // read hex string from 0th place to end before 'x'
            int ascii = (int)str[i];
            if(check_hex_param(str[i])) {
                if((str[i] >= 'A' && str[i] <= 'F') || (str[i] >= 'a' && str[i] <= 'f')) { 
                    // convert letters to numbers
                    ascii = hextodec(ascii);
                } else {
                    // convert number to its integer value
                    ascii = str[i] - '0';
                }
                counter = counter + (ascii * power(HEX_BASE, n));
                n++;
                endptr = &str + i;
            }
        }
    } else {
        // decimal
        for(int i = 0; str[i] != '\0' && (str[i] >= '0' && str[i] <= '9'); i++) {
            counter = counter * 10 + (str[i] - '0'); 
            endptr = &str + i;
        }
    }
    return counter;
}


