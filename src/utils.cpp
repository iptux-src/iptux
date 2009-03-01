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
#include "baling.h"
#include "output.h"
#include "Pal.h"

void my_delay(time_t sec, long nsec)
{
	struct timespec delay;

	delay.tv_sec = sec;
	delay.tv_nsec = nsec;
	nanosleep(&delay, NULL);
}

void ipv4_order(uint32_t *ip1, uint32_t *ip2)
{
	uint32_t ip;

	if (*ip1 > *ip2) {
		ip = *ip1;
		*ip1 = *ip2;
		*ip2 = ip;
	}
}

//只转换有效字符段
char *_iconv(const char *instr, const char *tocode, const char *fromcode)
{
	iconv_t cd;
	char *dst, *ptr;
	size_t size, len;

	cd = iconv_open(tocode, fromcode);
	if (cd == iconv_t(-1)) {
		pwarning(Fail,
			 _("act: create encode conversion table,warning: %s\n"),
			 strerror(errno));
		return Strdup(instr);
	}

	size = strlen(instr);
	len = size << 1;
	dst = ptr = (char *)Malloc(len + 1);
	iconv(cd, (char **)&instr, &size, &ptr, &len);
	iconv_close(cd);
	*ptr = '\0';

	return dst;
}

//向外传输 TRUE,向内传输 FALSE
char *transfer_encode(const char *instr, const char *encode, bool direc)
{
	if (strcasecmp(encode, "UTF-8") == 0)
		return Strdup(instr);
	if (direc)
		return _iconv(instr, encode, "UTF-8");
	return _iconv(instr, "UTF-8", encode);
}

pthread_t thread_create(ThreadFunc func, pointer data, bool joinable)
{
	pthread_t pid;
	int status;

	status = pthread_create(&pid, NULL, func, data);
	if (status != 0)
		pwarning(Quit, _("act: create a new thread,warning: %s\n"),
			 strerror(status));
	if (!joinable)
		pthread_detach(pid);

	return pid;
}

void get_file_system_info(const char *path, uint64_t *avail, uint64_t *total)
{
	struct statfs64 st;
	int result;

mark:	result = statfs64(path, &st);
	if (result == -1) {
		if (errno == EINTR)
			goto mark;
		*avail = *total = 0;
	} else {
		*avail = (uint64_t)st.f_bsize * st.f_bavail;
		*total = (uint64_t)st.f_bsize * st.f_blocks;
	}
}

char *my_getline(const char *str)
{
	const char *ptr;
	char *dst;
	size_t len;

	while (*str == '\x20' || *str == '\t')
		str++;

	if ( (len = (ptr = strpbrk(str, "\r\n")) ?
		     (ptr - str) : strlen(str))) {
		dst = (char *)Malloc(len + 1);
		memcpy(dst, str, len);
		*(dst + len) = '\0';
		return dst;
	}

	return NULL;
}

int strnchr(const char *str, char chr)
{
	int count;

	count = 0;
	while ( (str = strchr(str, chr)))
		str++, count++;

	return count;
}

void remove_foreach(pointer data, enum INFO_TYPE type)
{
	switch (type) {
	case PALINFO:
		delete(Pal *) data;
		break;
	case SYSICON:
		delete(SysIcon*) data;
		break;
	case FILEINFO:
		delete(FileInfo *) data;
		break;
	case CHIPDATA:
		delete(ChipData *) data;
		break;
	case NETSEGMENT:
		delete(NetSegment *) data;
		break;
	default:
		free(data);
		break;
	}
}

/*  为什么不能直接返回 src==dst ?  */
bool compare_foreach(uint32_t src, uint32_t dst)
{
	bool flag;

	flag = (src == dst);
	return flag;
}

char *getformattime(const char *format, ...)
{
	char *str1, *str2;
	va_list ap;
	time_t tt;

	va_start(ap, format);
	str1 = g_strdup_vprintf(format, ap);
	va_end(ap);

	time(&tt);
	str2 = g_strdup_printf("<%s> %s", str1, ctime(&tt));
	g_free(str1);

	return str2;
}

char *number_to_string_size(uint64_t number, bool rate)
{
	gchar *buf;

	if (number >= (1 << 30))
		buf =
		    g_strdup_printf("%.1fG\x20\x20", (float)number / (1 << 30));
	else if (number >= (1 << 20))
		buf =
		    g_strdup_printf("%.1fM\x20\x20", (float)number / (1 << 20));
	else if (number >= (1 << 10))
		buf =
		    g_strdup_printf("%.1fK\x20\x20", (float)number / (1 << 10));
	else
		buf = g_strdup_printf("%" PRIu64 "B\x20\x20", number);

	if (rate)
		strcpy(buf + strlen(buf) - 2, "/s");
	else
		*(buf + strlen(buf) - 2) = '\0';

	return buf;
}

const char *iptux_skip_string(const char *msg, size_t size, uint8_t times)
{
	const char *ptr;
	uint8_t count;

	ptr = msg, count = 0;
	while (count < times) {
		ptr += strlen(ptr) + 1;
		if ((size_t)(ptr - msg) < size)
			count++;
		else
			return NULL;
	}

	return ptr;
}

const char *iptux_skip_section(const char *msg, uint8_t times)
{
	const char *ptr;
	uint8_t count;

	ptr = msg, count = 0;
	while (count < times) {
		ptr = strchr(ptr, ':');
		if (ptr)
			ptr++;
		else
			return NULL;
		count++;
	}

	return ptr;
}

uint64_t iptux_get_hex64_number(const char *msg, uint8_t times)
{
	const char *ptr;
	uint64_t number;
	int result;

	if (!(ptr = iptux_skip_section(msg, times)))
		return 0;
	result = sscanf(ptr, "%" SCNx64, &number);
	if (result == 1)
		return number;
	return 0;
}

uint32_t iptux_get_dec_number(const char *msg, uint8_t times)
{
	const char *ptr;
	uint32_t number;
	int result;

	if (!(ptr = iptux_skip_section(msg, times)))
		return 0;
	result = sscanf(ptr, "%" SCNu32, &number);
	if (result == 1)
		return number;
	return 0;
}

uint32_t iptux_get_hex_number(const char *msg, uint8_t times)
{
	const char *ptr;
	uint32_t number;
	int result;

	if (!(ptr = iptux_skip_section(msg, times)))
		return 0;
	result = sscanf(ptr, "%" SCNx32, &number);
	if (result == 1)
		return number;
	return 0;
}

char *iptux_get_section_string(const char *msg, uint8_t times)
{
	const char *ptr, *pptr;
	char *string;
	size_t len;

	if (!(ptr = iptux_skip_section(msg, times)))
		return NULL;
	if ( (pptr = strchr(ptr, ':')))
		len = pptr - ptr;
	else
		len = strlen(ptr);
	string = (char *)Malloc(len + 1);
	memcpy(string, ptr, len);
	*(string + len) = '\0';

	return string;
}

char *ipmsg_get_filename(const char *msg, uint8_t times)
{
	static uint32_t serial = 0;
	const char *ptr;
	char filename[256];
	size_t len;

	if (!(ptr = iptux_skip_section(msg, times))) {
		snprintf(filename, 256, "iptux%" PRIu32, serial++);
		return Strdup(filename);
	}

	len = 0;
	while (*ptr != ':' || strncmp(ptr, "::", 2) == 0) {
		filename[len] = *ptr;
		len++;
		if (*ptr == ':') {
			memcpy((void *)ptr, "it", 2);
			ptr += 2;
		} else
			ptr++;
	}
	filename[len] = '\0';

	return Strdup(filename);
}

char *ipmsg_get_attach(const char *msg, uint8_t times)
{
	const char *ptr;

	if (!(ptr = iptux_skip_section(msg, times)))
		return NULL;
	return Strdup(ptr);
}

char *ipmsg_set_filename_pal(const char *pathname)
{
	const char *ptr;
	char filename[512];
	size_t len;

	ptr = strrchr(pathname, '/');
	ptr = ptr ? ptr + 1 : pathname;
	ptr = *ptr ? ptr : ptr - 1;
	len = 0;
	while (*ptr) {
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

	return Strdup(filename);
}

const char *ipmsg_set_filename_self(char *pathname)
{
	char *ptr;

	ptr = strrchr(pathname, '/');
	if (ptr && ptr != pathname)
		*ptr = '\0', ptr++;
	else
		ptr = pathname;

	return ptr;
}
