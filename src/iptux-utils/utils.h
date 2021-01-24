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
#include <memory>

#include <netinet/in.h>
#include <glib.h>

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

bool ipv4Equal(const in_addr& ip1, const in_addr& ip2);
int ipv4Compare(const in_addr& ip1, const in_addr& ip2);

std::string inAddrToString(in_addr ipv4);
in_addr inAddrFromString(const std::string& s);
uint32_t inAddrToUint32(in_addr ipv4);
in_addr inAddrFromUint32(uint32_t value);

template<typename ... Args>
std::string stringFormat( const char* format, Args ... args )
{
  gchar* buf = g_strdup_printf(format, args...);
  std::string res(buf, strlen(buf));
  g_free(buf);
  return res;
}

/**
 * @brief dump string like hexdump -C
 *
 * @param str source
 * @return std::string result
 */
std::string stringDump(const std::string& str);

/**
 * @brief dump string like C String
 *
 * @param str
 * @return std::string
 */
std::string stringDumpAsCString(const std::string& str);

class Helper {
 public:
  static void prepareDir(const std::string& fname);
};

ssize_t xwrite(int fd, const void *buf, size_t count);
ssize_t xread(int fd, void *buf, size_t count);
ssize_t read_ipmsg_prefix(int fd, void *buf, size_t count);
ssize_t read_ipmsg_filedata(int fd, void *buf, size_t count, size_t offset);
ssize_t read_ipmsg_dirfiles(int fd, void *buf, size_t count, size_t offset);
ssize_t read_ipmsg_fileinfo(int fd, void *buf, size_t count, size_t offset);

namespace utils {

/**
 * @brief calculate the file or directory size;
 *
 * return 0 if not exist.
 *
 * @param fileOrDirName
 * @return int64_t
 */
int64_t fileOrDirectorySize(const std::string& fileOrDirName);

}


}  // namespace iptux
#endif
