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
#ifndef IPTUX_UTILS_H
#define IPTUX_UTILS_H

#include <string>

#include "iptux/Models.h"

namespace iptux {

#define difftimeval(val2, val1)                                  \
  ((((val2).tv_sec - (val1).tv_sec) * 1000000 + (val2).tv_usec - \
    (val1).tv_usec) /                                            \
   1000000.0f)
#define percent(num1, num2) (100.0f * (num1) / (num2))

#define FLAG_ISSET(num, bit) ((num) & (1 << (bit)))
void FLAG_SET(uint8_t &num, int bit);
void FLAG_SET(uint8_t &num, int bit, bool value);
#define FLAG_CLR(num, bit) ((num) &= (~(1 << (bit))))

#define URL_REGEX                                          \
  "(http|ftp|https|sftp):\\/\\/[\\w\\-_]+(\\.[\\w\\-_]+)+" \
  "([\\w\\-\\.,@?^=%&amp;:/~\\+#]*[\\w\\-\\@?^=%&amp;/~\\+#])?"

#define NO_OPERATION_C \
  while (0)            \
    ;

typedef void *(*ThreadFunc)(void *);
void ipv4_order(in_addr_t *ip1, in_addr_t *ip2);

char *iptux_string_validate(const char *s, const std::string &codeset,
                            char **encode);
char *convert_encode(const char *string, const char *tocode,
                     const char *fromcode);
char *assert_filename_inexist(const char *path);
char *getformattime(gboolean date, const char *format, ...);

gboolean giter_compare_foreach(gunichar src, gunichar dst);

char *numeric_to_size(int64_t numeric);
char *numeric_to_rate(uint32_t numeric);
char *numeric_to_time(uint32_t numeric);

/* 以下函数调用的(ch)参数字符不应为('\0') */
const char *iptux_skip_string(const char *string, size_t size, uint8_t times);
const char *iptux_skip_section(const char *string, char ch, uint8_t times);
int64_t iptux_get_hex64_number(const char *msg, char ch, uint8_t times);
uint32_t iptux_get_hex_number(const char *msg, char ch, uint8_t times);
uint32_t iptux_get_dec_number(const char *msg, char ch, uint8_t times);
char *iptux_get_section_string(const char *msg, char ch, uint8_t times);
char *ipmsg_get_filename(const char *msg, char ch, uint8_t times);
char *ipmsg_get_attach(const char *msg, char ch, uint8_t times);
char *ipmsg_get_filename_pal(const char *pathname);
char *ipmsg_get_filename_me(const char *pathname, char **path);
char *iptux_erase_filename_suffix(const char *filename);
char *ipmsg_get_pathname_full(const char *path, const char *name);
std::string inAddrToString(in_addr_t ipv4);
in_addr_t stringToInAddr(const std::string& s);

}  // namespace iptux
#endif
