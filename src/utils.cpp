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

#include "utils.h"

#ifndef __APPLE__
  #include <sys/vfs.h>
#endif


/**
 * 对两个主机序的ipv4地址进行排序.
 * @param ip1 ipv4(host byte order)
 * @param ip2 ipv4(host byte order)
 */
void ipv4_order(in_addr_t *ip1, in_addr_t *ip2)
{
        in_addr_t ip;

        if (*ip1 > *ip2) {
                ip = *ip1;
                *ip1 = *ip2;
                *ip2 = ip;
        }
}

/**
 * 检查串(string)是否为有效的utf8串，若不是则根据字符集(codeset)来进行转码.
 * @param string 待检查的字符串
 * @param codeset 字符集编码，e.g.<gb18030,big5>
 * @retval encode 正确的字符集编码由此返回
 * @return 有效的utf8串
 * @note 若(string)为utf8编码，或(codeset)中不存在(string)的正确编码，函数将返回NULL
 * @see iptux_string_validate_copy()
 */
char *iptux_string_validate(const char *string, const char *codeset, char **encode)
{
        const char *pptr, *ptr;
        char *tstring, *cset;
        gsize rbytes, wbytes;

        *encode = NULL; //设置字符集编码未知
        tstring = NULL; //设置utf8有效串尚未成功获取
        if (!g_utf8_validate(string, -1, NULL) && codeset) {
                cset = NULL;
                ptr = codeset;
                do {
                        if (*(pptr = ptr + strspn(ptr, ",;\x20\t")) == '\0')
                                break;
                        if (!(ptr = strpbrk(pptr, ",;\x20\t")))
                                ptr = pptr + strlen(pptr);
                        g_free(cset);
                        cset = g_strndup(pptr, ptr - pptr);
                        if (strcasecmp(cset, "utf-8") == 0)
                                continue;
                } while (!(tstring = g_convert(string, -1, "utf-8", cset,
                                                 &rbytes, &wbytes, NULL)));
                *encode = cset;
        }

        return tstring;
}

/**
 * 检查串(string)是否为有效的utf8串，若不是则根据字符集(codeset)来进行转码.
 * @param string 待检查的字符串
 * @param codeset 字符集编码，e.g.<gb18030,big5>
 * @retval encode 正确的字符集编码由此返回
 * @return 有效的utf8串
 * @note 本函数无论处理结果如何，都会无条件的返回一个字符集编码以及一个新串
 * @see iptux_string_validate()
 */
char *iptux_string_validate_copy(const char *string, const char *codeset, char **encode)
{
        char *tstring;

        if (!codeset || !(tstring = iptux_string_validate(string, codeset, encode))) {
                *encode = g_strdup("utf-8");
                tstring = g_strdup(string);
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
char *convert_encode(const char *string, const char *tocode, const char *fromcode)
{
        gsize rbytes, wbytes;
        char *tstring;

        tstring = g_convert(string, -1, tocode, fromcode, &rbytes, &wbytes, NULL);

        return tstring;
}

/**
 * 转换字符串编码.
 * @param string 字符串
 * @param tocode 目标编码
 * @param fromcode 源编码
 * @return 新串
 * @note 无论函数处理结果如何，都将会返回一个新串
 */
char *convert_encode_copy(const char *string, const char *tocode, const char *fromcode)
{
        char *tstring;

        if (!(tstring = convert_encode(string, tocode, fromcode)))
                tstring = g_strdup(string);

        return tstring;
}

/**
 * 获取文件系统的存储空间信息.
 * @param path 文件路径
 * @retval avail 可用空间由此返回
 * @retval total 全部空间由此返回
 */
void get_file_system_info(const char *path, int64_t *avail, int64_t *total)
{
    #ifdef __APPLE__
        *avail = *total = 0;
    #else

        struct statfs64 st;
        int result;

mark:   switch (result = statfs64(path, &st)) {
        case 0:
                *avail = (int64_t)st.f_bsize * st.f_bavail;
                *total = (int64_t)st.f_bsize * st.f_blocks;
                break;
        default:
                if (errno == EINTR)
                        goto mark;
                *avail = *total = 0;
                break;
        }
    #endif
}

/**
 * 从字符串(str)当前位置开始提取一行数据.
 * @param str 字符串起始指针
 * @return 从(str)开始的一行数据
 * @note 首部、尾部的空白字符将会被删除
 */
char *iptux_string_getline(const char *str)
{
        const char *pptr, *ptr;
        char *dst;
        size_t len;

        dst = NULL;
        pptr = str + strspn(str, "\x20\t");
        ptr = strpbrk(pptr, "\r\n");
        if ( (len = ptr ? (ptr - pptr) : strlen(pptr))) {
                dst = g_strndup(pptr, len);
                g_strchomp(dst);
        }

        return dst;
}

/**
 * 确保(path)所指向的文件不存在.
 * @param path 文件路径
 * @return 新文件路径 *
 */
char *assert_filename_inexist(const char *path)
{
        char buf[MAX_PATHLEN];
        const char *ptr;
        uint16_t count;

        if (access(path, F_OK) != 0)
                return g_strdup(path);

        ptr = strrchr(path, '/');
        ptr = ptr ? ptr + 1 : path;
        count = 1;
        while (count) {
                snprintf(buf, MAX_PATHLEN, "%.*s%" PRIu16 "_%s",
                                 (int)(ptr - path), path, count, ptr);
                if (access(buf, F_OK) != 0)
                        break;
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
char *getformattime(gboolean date, const char *format, ...)
{
        char buf[MAX_BUFLEN], *msg, *ptr;
        struct tm *tm;
        time_t tt;
        va_list ap;

        va_start(ap, format);
        msg = g_strdup_vprintf(format, ap);
        va_end(ap);

        time(&tt);
        tm = localtime(&tt);
        if (date)
            strftime(buf, MAX_BUFLEN, "%c", tm);
        else
            strftime(buf, MAX_BUFLEN, "%X", tm);

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
gboolean giter_compare_foreach(gunichar src, gunichar dst)
{
        return (src == dst);
}

/**
 * 把数值(numeric)转换为文件长度描述串.
 * @param numeric 需要转换的数值
 * @return 描述串
 */
char *numeric_to_size(int64_t numeric)
{
        gchar *ptr;

        if (numeric >= ((int64_t)1 << 40))
                ptr = g_strdup_printf("%.1fT", (float)numeric / ((int64_t)1 << 40));
        else if (numeric >= (1 << 30))
                ptr = g_strdup_printf("%.1fG", (float)numeric / (1 << 30));
        else if (numeric >= (1 << 20))
                ptr = g_strdup_printf("%.1fM", (float)numeric / (1 << 20));
        else if (numeric >= (1 << 10))
                ptr = g_strdup_printf("%.1fK", (float)numeric / (1 << 10));
        else
                ptr = g_strdup_printf("%" PRId64 "B", numeric);

        return ptr;
}

/**
 * 把数值(numeric)转换为速度大小描述串.
 * @param numeric 需要转换的数值
 * @return 描述串
 */
char *numeric_to_rate(uint32_t numeric)
{
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
char *numeric_to_time(uint32_t numeric)
{
        uint32_t hour, minute;
        gchar *ptr;

        hour = numeric / 3600;
        numeric %= 3600;
        minute = numeric / 60;
        numeric %= 60;
        ptr = g_strdup_printf("%.2" PRIu32 ":%.2" PRIu32 ":%.2" PRIu32,
                                              hour, minute, numeric);
        return ptr;
}

/**
 * 查询以(string)为起始点，(times)个字符串后的指针位置.
 * @param string 串起始点
 * @param size 串有效长度
 * @param times 跳跃次数
 * @return 指针位置
 */
const char *iptux_skip_string(const char *string, size_t size, uint8_t times)
{
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
const char *iptux_skip_section(const char *string, char ch, uint8_t times)
{
        const char *ptr;
        uint8_t count;

        ptr = string;
        count = 0;
        while (count < times) {
                if (!(ptr = strchr(ptr, ch)))
                        break;
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
int64_t iptux_get_hex64_number(const char *msg, char ch, uint8_t times)
{
        const char *ptr;
        int64_t number;

        if (!(ptr = iptux_skip_section(msg, ch, times)))
                return 0;
        if (sscanf(ptr, "%" SCNx64, &number) == 1)
                return number;
        return 0;
}

/**
 * 读取以(msg)为起始点，跳跃(times)次(ch)字符后的16进制数值.
 * @param msg 串起始点
 * @param ch 分割字符
 * @param times 跳跃次数
 * @return 数值
 */
uint32_t iptux_get_hex_number(const char *msg, char ch, uint8_t times)
{
        const char *ptr;
        uint32_t number;

        if (!(ptr = iptux_skip_section(msg, ch, times)))
                return 0;
        if (sscanf(ptr, "%" SCNx32, &number) == 1)
                return number;
        return 0;
}

/**
 * 读取以(msg)为起始点，跳跃(times)次(ch)字符后的10进制数值.
 * @param msg 串起始点
 * @param ch 分割字符
 * @param times 跳跃次数
 * @return 数值
 */
uint32_t iptux_get_dec_number(const char *msg, char ch, uint8_t times)
{
        const char *ptr;
        uint32_t number;

        if (!(ptr = iptux_skip_section(msg, ch, times)))
                return 0;
        if (sscanf(ptr, "%" SCNu32, &number) == 1)
                return number;
        return 0;
}

/**
 * 读取以(msg)为起始点，跳跃(times)次(ch)字符后的一个字段.
 * @param msg 串起始点
 * @param ch 分割字符
 * @param times 跳跃次数
 * @return 新串
 */
char *iptux_get_section_string(const char *msg, char ch, uint8_t times)
{
        const char *pptr, *ptr;
        char *string;
        size_t len;

        if (!(pptr = iptux_skip_section(msg, ch, times)))
                return NULL;
        ptr = strchr(pptr, ch);
        if ((len = ptr ? ptr - pptr : strlen(pptr)) == 0)
                return NULL;
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
char *ipmsg_get_filename(const char *msg, char ch, uint8_t times)
{
        static uint32_t serial = 1;
        char filename[256];     //文件最大长度为255
        const char *ptr;
        size_t len;

        if ( (ptr = iptux_skip_section(msg, ch, times))) {
                len = 0;
                while (*ptr != ':' || strncmp(ptr, "::", 2) == 0) {
                        if (len < 255) {        //防止缓冲区溢出
                                filename[len] = *ptr;
                                len++;
                        }
                        if (*ptr == ':') {      //抹除分割符
                                memcpy((void *)ptr, "xx", 2);
                                ptr++;
                        }
                        ptr++;
                }
                filename[len] = '\0';

        } else
                snprintf(filename, 256, "%" PRIu32 "_file", serial++);

        return g_strdup(filename);
}

/**
 * 读取以(msg)为起始点，跳跃(times)次(ch)字符后的完整串.
 * @param msg 串起始点
 * @param ch 分割字符
 * @param times 跳跃次数
 * @return 新串
 */
char *ipmsg_get_attach(const char *msg, char ch, uint8_t times)
{
        const char *ptr;

        if (!(ptr = iptux_skip_section(msg, ch, times)))
                return NULL;
        return g_strdup(ptr);
}

/**
 * 从文件路径中分离出文件名，并转化为(IPMsg)格式的文件名.
 * @param pathname 文件路径
 * @return 文件名 *
 * @note 文件名特殊格式请参考IPMsg协议
 */
char *ipmsg_get_filename_pal(const char *pathname)
{
        char filename[512];     //文件最大长度为255
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
char *ipmsg_get_filename_me(const char *pathname, char **path)
{
        const char *ptr;
        char *file;

        ptr = strrchr(pathname, '/');
        if (ptr && ptr != pathname) {
                file = g_strdup(ptr + 1);
                if (path)
                        *path = g_strndup(pathname, ptr - pathname);
        } else {
                file = g_strdup(pathname);
                if (path)
                        *path = NULL;
        }

        return file;
}

/**
 * 从文件名中移除后缀.
 * @param filename 文件名
 * @return 文件 *
 */
char *iptux_erase_filename_suffix(const char *filename)
{
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
char *ipmsg_get_pathname_full(const char *path, const char *name)
{
    char filename[MAX_PATHLEN];
    strcpy(filename,path);
    strcat(filename,"/");
    strcat(filename,name);
    return g_strdup(filename);
}
