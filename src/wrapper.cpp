//
// C++ Implementation: wrapper
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "wrapper.h"
#include "deplib.h"

/**
 * 保证new运算符申请内存一定成功.
 * @param size 需要申请的内存大小
 * @return 新内存指针
 */
void *operator new(size_t size)
{
        return g_malloc(size);
}

/**
 * 写出数据.
 * @param fd 文件描述符
 * @param buf 缓冲区
 * @param count 缓冲区有效数据长度
 * @return 成功写出的数据长度
 */
ssize_t xwrite(int fd, const void *buf, size_t count)
{
        size_t offset;
        ssize_t size;

        size = -1;
        offset = 0;
        while ((offset != count) && (size != 0)) {
                if ((size = write(fd, (char *)buf + offset, count - offset)) == -1) {
                        if (errno == EINTR)
                                continue;
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
ssize_t xread(int fd, void *buf, size_t count)
{
        size_t offset;
        ssize_t size;

        size = -1;
        offset = 0;
        while ((offset != count) && (size != 0)) {
                if ((size = read(fd, (char *)buf + offset, count - offset)) == -1) {
                        if (errno == EINTR)
                                continue;
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
ssize_t read_ipmsg_prefix(int fd, void *buf, size_t count)
{
        uint number;
        size_t offset;
        ssize_t size;

        size = -1;
        offset = 0;
        number = 0;
        while ((offset != count) && (size != 0)) {
                if ((size = read(fd, (char *)buf + offset, count - offset)) == -1) {
                        if (errno == EINTR)
                                continue;
                        return -1;
                }
                offset += size;
                const char *endptr = (const char *)buf + offset;
                for (const char *curptr = endptr - size; curptr < endptr; ++curptr) {
                        if (*curptr == ':')
                                ++number;
                }
                if (number >= 5)
                        break;
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
ssize_t read_ipmsg_filedata(int fd, void *buf, size_t count, size_t offset)
{
        const char *curptr;
        uint number;
        ssize_t size;

        size = -1;
        number = 0;
        curptr = (const char *)buf;
        while ((offset != count) && (size != 0)) {
                const char *endptr = (const char *)buf + offset;
                for (; curptr < endptr; ++curptr) {
                        if (*curptr == ':')
                                ++number;
                }
                if (number > 2 || (number == 2 && *(curptr - 1) != ':'))
                        break;
                if ((size = read(fd, (char *)buf + offset, count - offset)) == -1) {
                        if (errno == EINTR)
                                continue;
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
ssize_t read_ipmsg_dirfiles(int fd, void *buf, size_t count, size_t offset)
{
        const char *curptr;
        uint number;
        ssize_t size;

        size = -1;
        number = 0;
        curptr = (const char *)buf;
        while ((offset != count) && (size != 0)) {
                const char *endptr = (const char *)buf + offset;
                for (; curptr < endptr; ++curptr) {
                        if (*curptr == ':')
                                ++number;
                }
                if (number > 1 || (number == 1 && *(curptr - 1) != ':'))
                        break;
                if ((size = read(fd, (char *)buf + offset, count - offset)) == -1) {
                        if (errno == EINTR)
                                continue;
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
ssize_t read_ipmsg_fileinfo(int fd, void *buf, size_t count, size_t offset)
{
        ssize_t size;
        uint32_t headsize;

        if (offset < count)     //注意不要写到缓冲区外了
                ((char *)buf)[offset] = '\0';
        while (!offset || !strchr((char *)buf, ':')
                 || sscanf((char *)buf, "%" SCNx32, &headsize) != 1
                 || headsize > offset) {
mark:           if ((size = read(fd, (char *)buf + offset, count - offset)) == -1) {
                        if (errno == EINTR)
                                goto mark;
                        return -1;
                } else if (size == 0)
                        return -1;
                if ((offset += size) == count)
                        break;
                ((char *)buf)[offset] = '\0';
        }

        return offset;
}
