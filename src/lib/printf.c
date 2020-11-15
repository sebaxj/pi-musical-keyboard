#include "printf.h"
#include "printf_internal.h"
#include "strings.h"
#include <stdarg.h>
#include "uart.h"

#define MAX_OUTPUT_LEN 1024

#define HEX_BASE (int)16

// helper function to calculate the length of an integer
static int int_length(int val) {
	int length = 0;
	while(val != 0) {
		val = val / 10;
		length++;
	}
	return length;
}

int unsigned_to_base(char *buf, size_t bufsize, unsigned int val, int base, int min_width)
{
	int int_len = int_length(val); int num_0 = 0;
	if(min_width > int_len) num_0 = min_width - int_len;

	int_len += num_0;
	char temp_buf[int_len + 1];
	temp_buf[int_len] = '\0';
	
	for(int i = 0; i < num_0; i++) {
		temp_buf[i] = '0';
	}

	for(int i = int_len - 1; val != 0; i--) {
		int dig = val % base;
		char ch;
		// if dig is between 0 and 9, convert to character between 0 and 9
		if(dig >= 0 && dig <= 9) {
			ch = '0' + dig;
		} else { // if dig is greater than 9, convert to HEX letter
			ch = 'a' + (dig - 10);
		}
		temp_buf[i] = ch;
		val = val / base;
	}

	int j;
	for(j = 0; (j < bufsize - 1) && temp_buf[j] != '\0'; j++) {
		buf[j] = temp_buf[j];
	}
	buf[j] = '\0';
	return int_len;
}

int signed_to_base(char *buf, size_t bufsize, int val, int base, int min_width)
{
	if(val < 0) {
		buf[0] = '-';
		buf[bufsize - 1] = '\0';
		buf++; bufsize--; min_width--; val = -val;
		
		int int_len = int_length(val);
		int num_0 = 0;
		if(min_width > int_len) num_0 = min_width - int_len;

		int_len += num_0;
		char temp_buf[int_len + 1];
		temp_buf[int_len] = '\0';
		
		for(int i = 0; i < num_0; i++) {
			temp_buf[i] = '0';
		}

		for(int i = int_len - 1; val != 0; i--) {
			int dig = val % base;
			char ch;
			// if dig is between 0 and 9, convert to character between 0 and 9
			if(dig >= 0 && dig <= 9) {
				ch = '0' + dig;
				
			} else { // if dig is greater than 9, convert to HEX letter
				ch = 'a' + (dig - 10);
			}
			temp_buf[i] = ch;
			val = val / base;
		}

		int j;
		for(j = 0; (j < bufsize - 1) && temp_buf[j] != '\0' && buf[j] != '\0'; j++) {
			buf[j] = temp_buf[j];
		}
		buf[j] = '\0';
		return int_len + 1;
	}
	return unsigned_to_base(buf, bufsize, val, base, min_width);
}

int vsnprintf(char *buf, size_t bufsize, const char *format, va_list args)
{
	int num = 0; char *str; char ch; unsigned int leading_zero = 0; int size = 0;

    for(format = format; *format != '\0' && size < bufsize; format++) { 
        while(*format != '%') { 
			if(*format == '\0') {
				buf[size] = '\0';
				return size;
			}
			if(size < bufsize - 1) buf[size] = *format;
			size++; format++;
        }

        format++; 
		if(*format == '\0') {
			buf[size] = '\0';
			return size;
		}
		switch(*format) {
			char temp_buf[MAX_OUTPUT_LEN];
			case '0': // padding needed

				// setup temp buffer to store converted string in before adding padding
				leading_zero = 0;
    			memset(temp_buf, 0x777, MAX_OUTPUT_LEN); // init contents with known value
				format++;

				// calculate padding needed
				while(*format != 'd' && *format != 'x') {
					if(*format == '\0') {
						buf[size] = '\0';
						return size;
					}
					leading_zero = leading_zero * 10 + (*format - '0');
					format++;
				}
				switch(*format) {
					case 'd': // decimal case with leading 0s
						num = signed_to_base(temp_buf, MAX_OUTPUT_LEN, va_arg(args, int), 10, 1);
						if(leading_zero - num > 0) {
							for(int i = leading_zero - num; i > 0; i--) {
								if(size < bufsize - 1) buf[size] = '0';
								size++;
							}
						}

						// add converted string to buf after leading 0s
						for(int i = 0; temp_buf[i] != '\0'; i++) {
							if(size < bufsize - 1) buf[size] = temp_buf[i];
							size++;
						}
						break;
					case 'x': // hex case with leading 0s
						num = unsigned_to_base(temp_buf, MAX_OUTPUT_LEN, va_arg(args, int), HEX_BASE, 1);
						if(leading_zero - num > 0) {
							for(int i = leading_zero - num; i >= 0; i--) {
								if(size < bufsize - 1) buf[size] = '0';
								size++;
							}
						}

						// add converted string to buf after leading 0s
						for(int i = 0; temp_buf[i] != '\0'; i++) {
							if(i != 0) { // junk value stored in index 0
								if(size < bufsize - 1) buf[size] = temp_buf[i];
								size++;
							}
						}
						break;
				}
				break;
			case 'd': // decimal (signed)
				num = signed_to_base(&buf[size], bufsize - size, va_arg(args, int), 10, 1);
				size += num;
				break;
			case 'x': // hex (unsigned)
				num = unsigned_to_base(&buf[size], bufsize - size, va_arg(args, int), HEX_BASE, 1);
				size += num;
				break;
			case '%': // output percentage sign
				if(size < bufsize - 1) buf[size] = '%';
				size++;
				break;
			case 's': // output string
				str = va_arg(args, char *);

				// evaluate string parameter
				while(*str != '\0') {
					if(size < bufsize - 1) buf[size] = *str;
					str++; size++;
				}
				break;
			case 'p': // pointer
				if(size < bufsize - 2) {
					buf[size] = '0'; size++;
					buf[size] = 'x'; size++;
				}
				int hex_addr = (int)va_arg(args, void *); // store pointer parameter as void

				// initialize temp buffer to store converted string
    			memset(temp_buf, 0x777, MAX_OUTPUT_LEN); // init contents with known value
				num = unsigned_to_base(temp_buf, MAX_OUTPUT_LEN, hex_addr, HEX_BASE, 8);

				// add converted string to buf after leading 0s
				for(int i = 0; temp_buf[i] != '\0'; i++) {
					if(i != 0) { // junk value stored in index 0
						if(size < bufsize - 1) buf[size] = temp_buf[i];
						size++;
					}
				}
				break;
			case 'c': // char
				ch = (char)va_arg(args, int);
				if(size < bufsize - 1) buf[size] = ch;
				size++;
				break;
		}  
    }
	buf[size] = '\0';

	// if size is greater than bufsize, but characters still remain, count them only
	for(format = format; *format != '\0'; format++) { 
        while(*format != '%') { 
			if(*format == '\0') {
				return size;
			}
			size++; format++;
        }

        format++; 
		if(*format == '\0') {
			return size;
		}
		switch(*format) {
			char temp_buf[MAX_OUTPUT_LEN];
			case '0': // padding needed

				// setup temp buffer to store converted string in before adding padding
				leading_zero = 0;
    			memset(temp_buf, 0x777, MAX_OUTPUT_LEN); // init contents with known value
				format++;

				// calculate padding needed
				while(*format != 'd' && *format != 'x') {
					if(*format == '\0') {
						return size;
					}
					leading_zero = leading_zero * 10 + (*format - '0');
					format++;
				}
				switch(*format) {
					case 'd': // decimal case with leading 0s
						num = signed_to_base(temp_buf, MAX_OUTPUT_LEN, va_arg(args, int), 10, 1);
						if(leading_zero - num > 0) {
							for(int i = leading_zero - num; i > 0; i--) {
								size++;
							}
						}

						// add converted string to buf after leading 0s
						for(int i = 0; temp_buf[i] != '\0'; i++) {
							size++;
						}
						break;
					case 'x': // hex case with leading 0s
						num = unsigned_to_base(temp_buf, MAX_OUTPUT_LEN, va_arg(args, int), HEX_BASE, 1);
						if(leading_zero - num > 0) {
							for(int i = leading_zero - num; i >= 0; i--) {
								size++;
							}
						}

						// add converted string to buf after leading 0s
						for(int i = 0; temp_buf[i] != '\0'; i++) {
							if(i != 0) { // junk value stored in index 0
								size++;
							}
						}
						break;
				}
				break;
			case 'd': // decimal (signed)
				num = signed_to_base(temp_buf, MAX_OUTPUT_LEN, va_arg(args, int), 10, 1);
				size += num;
				break;
			case 'x': // hex (unsigned)
				num = unsigned_to_base(temp_buf, MAX_OUTPUT_LEN, va_arg(args, int), HEX_BASE, 1);
				size += num;
				break;
			case '%': // output percentage sign
				size++;
				break;
			case 's': // output string
				str = va_arg(args, char *);

				// evaluate string parameter
				while(*str != '\0') {
					str++; size++;
				}
				break;
			case 'p': // pointers
				size++;
				size++;
				int hex_addr = (int)va_arg(args, void *); // store pointer parameter as void

				// initialize temp buffer to store converted string
    			memset(temp_buf, 0x777, MAX_OUTPUT_LEN); // init contents with known value
				num = unsigned_to_base(temp_buf, MAX_OUTPUT_LEN, hex_addr, HEX_BASE, 8);

				// add converted string to buf after leading 0s
				for(int i = 0; temp_buf[i] != '\0'; i++) {
					if(i != 0) { // junk value stored in index 0
						size++;
					}
				}
				break;
			case 'c': // char
				ch = (char)va_arg(args, int);
				size++;
				break;
		}  
	}
	return size; // number of characters that would be printed
}

int snprintf(char *buf, size_t bufsize, const char *format, ...)
{
	va_list arg;
	va_start(arg, format); // argument start

	int size = vsnprintf(buf, bufsize, format, arg);

	va_end(arg); // argument end
	return size;
}

int printf(const char *format, ...)
{
    char buffer[MAX_OUTPUT_LEN];
	va_list arg;

	va_start(arg, format);
	int size = vsnprintf(buffer, MAX_OUTPUT_LEN, format, arg);
	va_end (arg);
	uart_putstring(buffer);

	return size;
}
