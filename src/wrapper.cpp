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
 * 由于不方便确定ipmsg消息前缀的长度，所以我们这儿采取了时间限制的方法，
 * 也就是在距上一次为t的时间段里，如果没有数据到达，则认为消息前缀已经被
 * 完全读出。 \n
 * @param fd 文件描述符
 * @param buf 缓冲区
 * @param count 缓冲区长度
 * @return 成功读取的消息长度，-1表示读取消息出错
 */
ssize_t read_ipmsg_prefix(int fd, void *buf, size_t count)
{
	struct timeval tval;
	fd_set rset;
	size_t offset;
	ssize_t size;

	offset = 0;	//当前缓冲区已有字符数为0
	do {
		/* 先尝试着读取一次 */
mark:		if ((size = read(fd, (char *)buf + offset, count - offset)) == -1) {
			if (errno == EINTR)
				goto mark;
			return -1;
		}
		if ((offset += size) == count)	//如果已经读满则可以跳出循环了
			break;
		/* 为考察文件描述符做准备工作 */
		FD_ZERO(&rset);
		FD_SET(fd, &rset);
		tval.tv_sec = 0;
		tval.tv_usec = 100000;
	} while (size && (select(fd + 1, &rset, NULL, NULL, &tval) == 1));

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

	if (offset < count)	//注意不要写到缓冲区外了
		((char *)buf)[offset] = '\0';
	while (!offset || !strchr((char *)buf, ':')
		 || sscanf((char *)buf, "%" SCNx32, &headsize) != 1
		 || headsize > offset) {
mark:		if ((size = read(fd, (char *)buf + offset, count - offset)) == -1) {
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
