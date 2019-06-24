//
// C++ Implementation: utils
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "config.h"
#include "utils.h"

#include <cerrno>
#include <cinttypes>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/mount.h>
#ifndef __APPLE__
#include <sys/vfs.h>
#endif

#include "iptux-core/ipmsg.h"
#include "iptux-core/output.h"
#include "Exception.h"

using namespace std;

namespace iptux {

/**
 * 检查串(string)是否为有效的utf8串，若不是则根据字符集(codeset)来进行转码.
 * @param string 待检查的字符串
 * @param codeset 字符集编码，e.g.<gb18030,big5>
 * @retval encode 正确的字符集编码由此返回
 * @return 有效的utf8串
 * @note
 * 若(string)为utf8编码，或(codeset)中不存在(string)的正确编码，函数将返回NULL
 * @see iptux_string_validate_copy()
 */
char *iptux_string_validate(const char *s, const string &codeset,
                            char **encode) {
  const char *pptr, *ptr;
  char *tstring, *cset;
  gsize rbytes, wbytes;

  *encode = NULL;  //设置字符集编码未知
  tstring = NULL;  //设置utf8有效串尚未成功获取
  if (!g_utf8_validate(s, -1, NULL) && !codeset.empty()) {
    cset = NULL;
    ptr = codeset.c_str();
    do {
      if (*(pptr = ptr + strspn(ptr, ",;\x20\t")) == '\0') break;
      if (!(ptr = strpbrk(pptr, ",;\x20\t"))) ptr = pptr + strlen(pptr);
      g_free(cset);
      cset = g_strndup(pptr, ptr - pptr);
      if (strcasecmp(cset, "utf-8") == 0) continue;
    } while (
        !(tstring = g_convert(s, -1, "utf-8", cset, &rbytes, &wbytes, NULL)));
    *encode = cset;
  }

  return tstring;
}

/**
 * 转换字符串编码.
 * @param string 字符串
 * @param tocode 目标编码
 * @param fromcode 源编码
 * @return 新串
 * @note 若函数执行出错，将返回NULL
 */
char *convert_encode(const char *string, const char *tocode,
                     const char *fromcode) {
  gsize rbytes, wbytes;
  char *tstring;
  GError* err = nullptr;

  tstring = g_convert(string, -1, tocode, fromcode, &rbytes, &wbytes, &err);
  if (err != nullptr) {
    LOG_INFO("g_convert failed: %s-%d-%s", g_quark_to_string(err->domain), err->code, err->message);
    g_clear_error(&err);
    return nullptr;
  }
  return tstring;
}

/**
 * 确保(path)所指向的文件不存在.
 * @param path 文件路径
 * @return 新文件路径 *
 */
char *assert_filename_inexist(const char *path) {
  char buf[MAX_PATHLEN];
  const char *ptr;
  uint16_t count;

  if (access(path, F_OK) != 0) return g_strdup(path);

  ptr = strrchr(path, '/');
  ptr = ptr ? ptr + 1 : path;
  count = 1;
  while (count) {
    snprintf(buf, MAX_PATHLEN, "%.*s%" PRIu16 "_%s", (int)(ptr - path), path,
             count, ptr);
    if (access(buf, F_OK) != 0) break;
    count++;
  }

  /* 概率极低，不妨忽略错误情形 */
  return g_strdup(buf);
}

/**
 * 获取包含指定数据的格式化时间串.
 * @param date 是否需要包含日期
 * @param format as in printf()
 * @param ... as in printf()
 * @return 时间串
 */
char *getformattime(gboolean date, const char *format, ...) {
  char buf[MAX_BUFLEN], *msg, *ptr;
  time_t tt;
  va_list ap;

  va_start(ap, format);
  msg = g_strdup_vprintf(format, ap);
  va_end(ap);

  time(&tt);
  struct tm tm;
  localtime_r(&tt, &tm);
  if (date)
    strftime(buf, MAX_BUFLEN, "%c", &tm);
  else
    strftime(buf, MAX_BUFLEN, "%X", &tm);

  ptr = g_strdup_printf("(%s) %s:", buf, msg);
  g_free(msg);

  return ptr;
}

/**
 * 对GtkTextBuffer的迭代器(GtkTextIter)所指的字符进行比较.
 * @param src 源字符
 * @param dst 目标字符
 * @return Gtk+库
 */
gboolean giter_compare_foreach(gunichar src, gunichar dst) {
  return (src == dst);
}

/**
 * 把数值(numeric)转换为文件长度描述串.
 * @param numeric 需要转换的数值
 * @return 描述串
 */
char *numeric_to_size(int64_t numeric) {
  gchar *ptr;

  if (numeric >= ((int64_t)1 << 40))
    ptr = g_strdup_printf("%.1fTiB", (double)numeric / ((int64_t)1 << 40));
  else if (numeric >= (1 << 30))
    ptr = g_strdup_printf("%.1fGiB", (double)numeric / (1 << 30));
  else if (numeric >= (1 << 20))
    ptr = g_strdup_printf("%.1fMiB", (double)numeric / (1 << 20));
  else if (numeric >= (1 << 10))
    ptr = g_strdup_printf("%.1fKiB", (double)numeric / (1 << 10));
  else
    ptr = g_strdup_printf("%" PRId64 "B", numeric);

  return ptr;
}

/**
 * 把数值(numeric)转换为速度大小描述串.
 * @param numeric 需要转换的数值
 * @return 描述串
 */
char *numeric_to_rate(uint32_t numeric) {
  gchar *ptr;

  if (numeric >= (1 << 30))
    ptr = g_strdup_printf("%.1fG/s", (float)numeric / (1 << 30));
  else if (numeric >= (1 << 20))
    ptr = g_strdup_printf("%.1fM/s", (float)numeric / (1 << 20));
  else if (numeric >= (1 << 10))
    ptr = g_strdup_printf("%.1fK/s", (float)numeric / (1 << 10));
  else
    ptr = g_strdup_printf("%" PRIu32 "B/s", numeric);

  return ptr;
}

/**
 * 把数值(numeric)转换为时间长度描述串.
 * @param numeric 需要转换的数值
 * @return 描述串
 */
char *numeric_to_time(uint32_t numeric) {
  uint32_t hour, minute;
  gchar *ptr;

  hour = numeric / 3600;
  numeric %= 3600;
  minute = numeric / 60;
  numeric %= 60;
  ptr = g_strdup_printf("%.2" PRIu32 ":%.2" PRIu32 ":%.2" PRIu32, hour, minute,
                        numeric);
  return ptr;
}

/**
 * 查询以(string)为起始点，(times)个字符串后的指针位置.
 * @param string 串起始点
 * @param size 串有效长度
 * @param times 跳跃次数
 * @return 指针位置
 */
const char *iptux_skip_string(const char *string, size_t size, uint8_t times) {
  const char *ptr;
  uint8_t count;

  ptr = string;
  count = 0;
  while (count < times) {
    ptr += strlen(ptr) + 1;
    if ((size_t)(ptr - string) >= size) {
      ptr = NULL;
      break;
    }
    count++;
  }

  return ptr;
}

/**
 * 查询以(string)为起始点，跳跃(times)次(ch)字符后的指针位置.
 * @param string 串起始点
 * @param ch 分割字符
 * @param times 跳跃次数
 * @return 指针位置
 */
const char *iptux_skip_section(const char *string, char ch, uint8_t times) {
  const char *ptr;
  uint8_t count;

  ptr = string;
  count = 0;
  while (count < times) {
    if (!(ptr = strchr(ptr, ch))) break;
    ptr++;  //跳过当前分割字符
    count++;
  }

  return ptr;
}

/**
 * 读取以(msg)为起始点，跳跃(times)次(ch)字符后的16进制数值.
 * @param msg 串起始点
 * @param ch 分割字符
 * @param times 跳跃次数
 * @return 数值
 */
int64_t iptux_get_hex64_number(const char *msg, char ch, uint8_t times) {
  const char *ptr;
  int64_t number;

  if (!(ptr = iptux_skip_section(msg, ch, times))) return 0;
  if (sscanf(ptr, "%" SCNx64, &number) == 1) return number;
  return 0;
}

/**
 * 读取以(msg)为起始点，跳跃(times)次(ch)字符后的16进制数值.
 * @param msg 串起始点
 * @param ch 分割字符
 * @param times 跳跃次数
 * @return 数值
 */
uint32_t iptux_get_hex_number(const char *msg, char ch, uint8_t times) {
  const char *ptr;
  uint32_t number;

  if (!(ptr = iptux_skip_section(msg, ch, times))) return 0;
  if (sscanf(ptr, "%" SCNx32, &number) == 1) return number;
  return 0;
}

/**
 * 读取以(msg)为起始点，跳跃(times)次(ch)字符后的10进制数值.
 * @param msg 串起始点
 * @param ch 分割字符
 * @param times 跳跃次数
 * @return 数值
 */
uint32_t iptux_get_dec_number(const char *msg, char ch, uint8_t times) {
  const char *ptr;
  uint32_t number;

  if (!(ptr = iptux_skip_section(msg, ch, times))) return 0;
  if (sscanf(ptr, "%" SCNu32, &number) == 1) return number;
  return 0;
}

/**
 * 读取以(msg)为起始点，跳跃(times)次(ch)字符后的一个字段.
 * @param msg 串起始点
 * @param ch 分割字符
 * @param times 跳跃次数
 * @return 新串
 */
char *iptux_get_section_string(const char *msg, char ch, uint8_t times) {
  const char *pptr, *ptr;
  char *string;
  size_t len;

  if (!(pptr = iptux_skip_section(msg, ch, times))) return NULL;
  ptr = strchr(pptr, ch);
  if ((len = ptr ? ptr - pptr : strlen(pptr)) == 0) return NULL;
  string = g_strndup(pptr, len);

  return string;
}

/**
 * 读取以(msg)为起始点，跳跃(times)次(ch)字符后的一个文件名.
 * @param msg 串起始点
 * @param ch 分割字符
 * @param times 跳跃次数
 * @return 新串 *
 * @note 文件名特殊格式请参考IPMsg协议
 * @note (msg)串会被修改
 */
char *ipmsg_get_filename(const char *msg, char ch, uint8_t times) {
  char filename[256];  //文件最大长度为255
  const char *ptr;

  if ((ptr = iptux_skip_section(msg, ch, times))) {
    size_t len = 0;
    while (*ptr != ':' || strncmp(ptr, "::", 2) == 0) {
      if (len < 255) {  //防止缓冲区溢出
        filename[len] = *ptr;
        len++;
      }
      if (*ptr == ':') {  //抹除分割符
        memcpy((void *)ptr, "xx", 2);
        ptr++;
      }
      ptr++;
    }
    filename[len] = '\0';

  } else {
    static uint32_t serial = 1;
    snprintf(filename, 256, "%" PRIu32 "_file", serial++);
  }

  return g_strdup(filename);
}

/**
 * 读取以(msg)为起始点，跳跃(times)次(ch)字符后的完整串.
 * @param msg 串起始点
 * @param ch 分割字符
 * @param times 跳跃次数
 * @return 新串
 */
char *ipmsg_get_attach(const char *msg, char ch, uint8_t times) {
  const char *ptr;

  if (!(ptr = iptux_skip_section(msg, ch, times))) return NULL;
  return g_strdup(ptr);
}

/**
 * 从文件路径中分离出文件名，并转化为(IPMsg)格式的文件名.
 * @param pathname 文件路径
 * @return 文件名 *
 * @note 文件名特殊格式请参考IPMsg协议
 */
char *ipmsg_get_filename_pal(const char *pathname) {
  char filename[512];  //文件最大长度为255
  const char *ptr;
  size_t len;

  ptr = strrchr(pathname, '/');
  ptr = ptr ? ptr + 1 : pathname;

  len = 0;
  while (*ptr && len < 510) {
    if (*ptr == ':') {
      memcpy(filename + len, "::", 2);
      len += 2;
    } else {
      filename[len] = *ptr;
      len++;
    }
    ptr++;
  }
  filename[len] = '\0';

  return g_strdup(filename);
}

/**
 * 从文件路径中分离出文件名以及路径.
 * @param pathname 文件路径
 * @retval path 路径由此返回
 * @return 文件名 *
 * @note 路径可能为NULL
 */
char *ipmsg_get_filename_me(const char *pathname, char **path) {
  const char *ptr;
  char *file;

  ptr = strrchr(pathname, '/');
  if (ptr && ptr != pathname) {
    file = g_strdup(ptr + 1);
    if (path) *path = g_strndup(pathname, ptr - pathname);
  } else {
    file = g_strdup(pathname);
    if (path) *path = NULL;
  }

  return file;
}

/**
 * 从文件名中移除后缀.
 * @param filename 文件名
 * @return 文件 *
 */
char *iptux_erase_filename_suffix(const char *filename) {
  const char *ptr;
  char *file;

  ptr = strrchr(filename, '.');
  if (ptr && ptr != filename)
    file = g_strndup(filename, ptr - filename);
  else
    file = g_strdup(filename);

  return file;
}
/**
 * 从给定的文件路径和文件名返回全文件名.
 * @param path  文件路径
 * @param name  文件名
 * @return 带路径的文件名 *
 */
char *ipmsg_get_pathname_full(const char *path, const char *name) {
  char filename[MAX_PATHLEN];
  strcpy(filename, path);
  strcat(filename, "/");
  strcat(filename, name);
  return g_strdup(filename);
}

void FLAG_SET(uint8_t &num, int bit) { ((num) |= (1 << (bit))); }

void FLAG_SET(uint8_t &num, int bit, bool value) {
  if (value) {
    ((num) |= (1 << (bit)));
  } else {
    ((num) &= (~(1 << (bit))));
  }
}

std::string inAddrToString(in_addr inAddr) {
  ostringstream oss;
  auto ipv4 = inAddr.s_addr;
  oss << int(ipv4 & 0xff) << "."
      << int((ipv4 & 0xff00) >> 8) << "."
      << int((ipv4 & 0xff0000) >> 16) << "."
      << int((ipv4 & 0xff000000) >> 24);
  return oss.str();
}

in_addr inAddrFromString(const std::string& s) {
  in_addr res;
  if(inet_pton(AF_INET, s.c_str(), &res.s_addr) == 1) {
    return res;
  }
  throw Exception(ErrorCode::INVALID_IP_ADDRESS);
}

std::string stringDump(const std::string& str) {
  if(str.empty()) {
    return "";
  }

  ostringstream oss;
  for(int i = 0; i < int(str.size()); i+= 16) {
    oss << stringFormat("%08x  ", i);
    for(int j = 0; j < 8; ++j) {
      if(i+j < int(str.size())) {
        oss << stringFormat("%02x ", uint8_t(str[i+j]));
      } else {
        oss << "   ";
      }
    }
    oss << ' ';
    for(int j = 8; j < 16; ++j) {
      if(i+j < int(str.size())) {
        oss << stringFormat("%02x ", uint8_t(str[i+j]));
      } else {
        oss << "   ";
      }
    }
    oss << " |";
    for(int j = 0; j < 16; ++j) {
      if(i+j >= str.size()) {
        break;
      }
      char c = str[i+j];
      if(c >= ' ' && c <= '\x7e') {
        oss << c;
      } else {
        oss << '.';
      }
    }
    oss << "|\n";
  }
  oss << stringFormat("%08x\n", str.size());;
  return oss.str();
}


std::string stringDumpAsCString(const std::string& str) {
  static const char* tables[32] = {
    "\\x00", "\\x01", "\\x02", "\\x03", "\\x04", "\\x05", "\\x06", "\\x07",
    "\\x08", "\\t",   "\\n",   "\\x0b", "\\x0c", "\\r",   "\\x0e", "\\x0f",
    "\\x10", "\\x11", "\\x12", "\\x13", "\\x14", "\\x15", "\\x16", "\\x17",
    "\\x18", "\\x19", "\\x1a", "\\x1b", "\\x1c", "\\x1d", "\\x1e", "\\x1f"
  };

  ostringstream oss;
  oss << '"';
  for(int i = 0; i < int(str.size()); ++i) {
    if(uint8_t(str[i]) < 0x20) {
      oss << tables[str[i]];
    } else if (str[i] == '"') {
      oss << "\\\"";
    } else if (str[i] == '\\') {
      oss << "\\\\";
    } else if (uint8_t(str[i]) < 0x7f) {
      oss << str[i];
    } else {
      oss << stringFormat("\\x%02x", uint8_t(str[i]));
    }
  }
  oss << '"';
  return oss.str();
}

void Helper::prepareDir(const std::string& fname) {
  auto path = g_path_get_dirname(fname.c_str());
  if(g_mkdir_with_parents(path, 0755) != 0) {
    LOG_ERROR("g_mkdir_with_parents failed: %s, %s", path, strerror(errno));
  }
  g_free(path);
}

/**
 * 写出数据.
 * @param fd 文件描述符
 * @param buf 缓冲区
 * @param count 缓冲区有效数据长度
 * @return 成功写出的数据长度
 */
ssize_t xwrite(int fd, const void *buf, size_t count) {
  size_t offset;
  ssize_t size;

  size = -1;
  offset = 0;
  while ((offset != count) && (size != 0)) {
    if ((size = write(fd, (char *)buf + offset, count - offset)) == -1) {
      if (errno == EINTR) continue;
      return -1;
    }
    offset += size;
  }

  return offset;
}

/**
 * 读取数据.
 * @param fd 文件描述符
 * @param buf 缓冲区
 * @param count 缓冲区长度
 * @return 成功读取的数据长度
 */
ssize_t xread(int fd, void *buf, size_t count) {
  size_t offset;
  ssize_t size;

  size = -1;
  offset = 0;
  while ((offset != count) && (size != 0)) {
    if ((size = read(fd, (char *)buf + offset, count - offset)) == -1) {
      if (errno == EINTR) continue;
      return -1;
    }
    offset += size;
  }

  return offset;
}

/**
 * 读取ipmsg消息前缀.
 * Ver(1):PacketNo:SenderName:SenderHost:CommandNo:AdditionalSection.\n
 * @param fd 文件描述符
 * @param buf 缓冲区
 * @param count 缓冲区长度
 * @return 成功读取的消息长度，-1表示读取消息出错
 */
ssize_t read_ipmsg_prefix(int fd, void *buf, size_t count) {
  uint number;
  size_t offset;
  ssize_t size;

  size = -1;
  offset = 0;
  number = 0;
  while ((offset != count) && (size != 0)) {
    if ((size = read(fd, (char *)buf + offset, count - offset)) == -1) {
      if (errno == EINTR) continue;
      return -1;
    }
    offset += size;
    const char *endptr = (const char *)buf + offset;
    for (const char *curptr = endptr - size; curptr < endptr; ++curptr) {
      if (*curptr == ':') ++number;
    }
    if (number >= 5) break;
  }

  return offset;
}

/**
 * 读取ipmsg文件请求消息前缀.
 * packetID:fileID:offset.\n
 * @param fd 文件描述符
 * @param buf 缓冲区
 * @param count 缓冲区长度
 * @param offset 缓冲区无效数据偏移量
 * @return 成功读取的消息长度，-1表示读取消息出错
 */
ssize_t read_ipmsg_filedata(int fd, void *buf, size_t count, size_t offset) {
  const char *curptr;
  uint number;
  ssize_t size;

  size = -1;
  number = 0;
  curptr = (const char *)buf;
  while ((offset != count) && (size != 0)) {
    const char *endptr = (const char *)buf + offset;
    for (; curptr < endptr; ++curptr) {
      if (*curptr == ':') ++number;
    }
    if (number > 2 || (number == 2 && *(curptr - 1) != ':')) break;
    if ((size = read(fd, (char *)buf + offset, count - offset)) == -1) {
      if (errno == EINTR) continue;
      return -1;
    }
    offset += size;
  }

  return offset;
}

/**
 * 读取ipmsg目录请求消息前缀.
 * packetID:fileID.\n
 * @param fd 文件描述符
 * @param buf 缓冲区
 * @param count 缓冲区长度
 * @param offset 缓冲区无效数据偏移量
 * @return 成功读取的消息长度，-1表示读取消息出错
 */
ssize_t read_ipmsg_dirfiles(int fd, void *buf, size_t count, size_t offset) {
  const char *curptr;
  uint number;
  ssize_t size;

  size = -1;
  number = 0;
  curptr = (const char *)buf;
  while ((offset != count) && (size != 0)) {
    const char *endptr = (const char *)buf + offset;
    for (; curptr < endptr; ++curptr) {
      if (*curptr == ':') ++number;
    }
    if (number > 1 || (number == 1 && *(curptr - 1) != ':')) break;
    if ((size = read(fd, (char *)buf + offset, count - offset)) == -1) {
      if (errno == EINTR) continue;
      return -1;
    }
    offset += size;
  }

  return offset;
}

/**
 * 读取ipmsg文件头信息.
 * 本函数的退出条件为: \n
 * 1.缓冲区内必须要有数据; \n
 * 2.文件头长度必须能够被获得; \n
 * 3.文件头长度必须小于或等于缓冲区内已有数据长度; \n
 * 4.读取数据出错(晕，这还值得怀疑吗？). \n
 * @param fd 文件描述符
 * @param buf 缓冲区
 * @param count 缓冲区长度
 * @param offset 缓冲区无效数据偏移量
 * @return 成功读取的信息长度
 */
ssize_t read_ipmsg_fileinfo(int fd, void *buf, size_t count, size_t offset) {
  ssize_t size;
  uint32_t headsize;

  if (offset < count)  //注意不要写到缓冲区外了
    ((char *)buf)[offset] = '\0';
  while (!offset || !strchr((char *)buf, ':') ||
         sscanf((char *)buf, "%" SCNx32, &headsize) != 1 || headsize > offset) {
  mark:
    if ((size = read(fd, (char *)buf + offset, count - offset)) == -1) {
      if (errno == EINTR) goto mark;
      return -1;
    } else if (size == 0)
      return -1;
    if ((offset += size) == count) break;
    ((char *)buf)[offset] = '\0';
  }

  return offset;
}

int ipv4Compare(const in_addr& ip1, const in_addr& ip2) {
  uint32_t i1 = inAddrToUint32(ip1);
  uint32_t i2 = inAddrToUint32(ip2);
  if(i1 < i2) {
    return -1;
  }
  if(i1 == i2) {
    return 0;
  }
  return 1;
}

bool ipv4Equal(const in_addr& ip1, const in_addr& ip2) {
  return ip1.s_addr == ip2.s_addr;
}

uint32_t inAddrToUint32(in_addr ipv4) {
  return ntohl(ipv4.s_addr);
}

in_addr inAddrFromUint32(uint32_t value) {
  in_addr res;
  res.s_addr = htonl(value);
  return res;
}


}  // namespace iptux
