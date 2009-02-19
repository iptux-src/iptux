//
// C++ Interface: utils
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef UTILS_H
#define UTILS_H

#include "udt.h"

#define ipv4_order(digit1,digit2) { \
	asm("cmpl %%eax,%%ebx; \
	jge mark; \
	movl %%eax,%%edx; \
	movl %%ebx,%%eax; \
	movl %%edx,%%ebx; \
	mark: nop"\
	:"=a" (digit1),"=b" (digit2)\
	:"a" (digit1),"b" (digit2) \
	); \
}
#define difftimeval(val2,val1) \
	((((val2).tv_sec-(val1).tv_sec)*1000000 \
	+ (val2).tv_usec-(val1).tv_usec) \
	/ 1000000.0f)
#define percent(num1,num2) (100.0f*(num1)/(num2))

#define FLAG_ISSET(num,bit) ((num)&(1<<(bit)))
#define FLAG_SET(num,bit) ((num)|=(1<<(bit)))
#define FLAG_CLR(num,bit) ((num)&=(~(1<<(bit))))

void my_delay(time_t sec, long nsec);
char *_iconv(const char *instr, const char *tocode, const char *fromcode);
char *transfer_encode(const char *instr, const char *encode, bool direc);

typedef void *(*ThreadFunc) (void *);
pthread_t thread_create(ThreadFunc func, pointer data, bool joinable);
void get_file_system_info(const char *path, uint64_t *avail, uint64_t *total);
char *my_getline(const char *str);
int strnchr(const char *str, char chr);
void remove_foreach(pointer data, enum INFO_TYPE type);
bool compare_foreach(uint32_t src, uint32_t dst);
char *getformattime(const char *format, ...);
char *number_to_string_size(uint64_t number, bool rate = false);

const char *iptux_skip_string(const char *msg, size_t size, uint8_t times);
const char *iptux_skip_section(const char *msg, uint8_t times);
uint64_t iptux_get_hex64_number(const char *msg, uint8_t times);
uint32_t iptux_get_dec_number(const char *msg, uint8_t times);
uint32_t iptux_get_hex_number(const char *msg, uint8_t times);
char *iptux_get_section_string(const char *msg, uint8_t times);
char *ipmsg_get_filename(const char *msg, uint8_t times);
char *ipmsg_get_attach(const char *msg, uint8_t times);
char *ipmsg_set_filename_pal(const char *pathname);
const char *ipmsg_set_filename_self(char *pathname);

#endif
