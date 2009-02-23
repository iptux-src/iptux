//
// C++ Implementation: baling
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "baling.h"
#include "Control.h"
#include "output.h"
#include "utils.h"
#include "net.h"

char *Strdup(const char *str)
{
	char *dst;
	size_t len;

	len = strlen(str) + 1;
	dst = (char *)Malloc(len);
	memcpy(dst, str, len);

	return dst;
}

void *Malloc(size_t size)
{
	void *dst;

	dst = malloc(size);
	if (!dst)
		pwarning(Quit, _("act: allocate memory [%" PRIu32 "],"
				   "warning: %s\n"), size, strerror(errno));

	return dst;
}

void *operator      new(size_t size)
{
	return Malloc(size);
}

int Socket(int domain, int type, int protocol)
{
	int sock;

	//<sys/select.h> FD_SETSIZE = 1024
	sock = socket(domain, type, protocol);
	if (sock == -1)
		pwarning(Quit, _("act: create socket,warning: %s\n"),
			 strerror(errno));

	return sock;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t * addr_len)
{
	int subsock;

 mark:	subsock = accept(sockfd, addr, addr_len);
	if (subsock == -1) {
		if (errno == EINTR)
			goto mark;
		pwarning(Fail,
			 _("act: TCP accepts a new connect (%d),warning: %s\n"),
			 sockfd, strerror(errno));
	}

	return subsock;
}

int Connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{
	char ipstr[INET_ADDRSTRLEN];
	int status;

 mark:	status = connect(sockfd, serv_addr, addrlen);
	if (status == -1) {
		if (errno == EINTR)
			goto mark;
		inet_ntop(AF_INET, &((SI *) serv_addr)->sin_addr, ipstr,
			  INET_ADDRSTRLEN);
		pwarning(Fail, _("act: connect to server '%s',warning: %s\n"),
			 ipstr, strerror(errno));
	}

	return status;
}

int Stat(const char *path, struct stat64 *buf)
{
	int result;

	result = stat64(path, buf);
	if (result == -1)
		pwarning(Fail,
			 _("act: look up file's status '%s',warning: %s\n"),
			 path, strerror(errno));

	return result;
}

int Open(const char *pathname, int flags, ...)
{
	va_list ap;
	int fd;

	if ((flags & O_ACCMODE) == O_RDONLY) {
		fd = open(pathname, flags);
		if (fd == -1)
			pwarning(Fail, _("act: open file '%s',warning: %s\n"),
				 pathname, strerror(errno));
	} else {
		va_start(ap, flags);
		fd = open(pathname, flags, va_arg(ap, mode_t));
		va_end(ap);
		if (fd == -1)
			pwarning(Fail, _("act: create file '%s',warning: %s\n"),
				 pathname, strerror(errno));
	}

	return fd;
}

ssize_t Read(int fd, void *buf, size_t count)
{
	size_t len;
	ssize_t size;

	len = 0;
	do {
		size = read(fd, (char *)buf + len, count - len);
		if (size == -1) {
			if (errno == EINTR)
				continue;
			pwarning(Fail, _("act: read data (%d)[%" PRIu32 "],"
				 "warning: %s\n"), fd, count, strerror(errno));
			return -1;
		}
		len += size;
	}
	while (len != count && size);

	return len;
}

ssize_t Write(int fd, const void *buf, size_t count)
{
	size_t len;
	ssize_t size;

	len = 0;
	do {
		size = write(fd, (char *)buf + len, count - len);
		if (size == -1) {
			if (errno == EINTR)
				continue;
			pwarning(Fail, _("act: write data (%d)[%" PRIu32 "],"
				 "warning: %s\n"), fd, count, strerror(errno));
			return -1;
		}
		len += size;
	}
	while (len != count && size);

	return len;
}

ssize_t read_ipmsg_prefix(int fd, void *buf, size_t count, uint8_t times)
{
	size_t len;
	ssize_t size;

	((char *)buf)[0] = '\0', len = 0;
	while (strnchr((char *)buf, ':') < times) {
		size = read(fd, (char *)buf + len, count - len);
		if (size == -1) {
			if (errno == EINTR)
				continue;
			pwarning(Fail, _("act: read data (%d),warning: %s\n"),
				 fd, strerror(errno));
			return -1;
		} else if (size == 0)
			return -1;
		if ((len += size) < count)
			((char *)buf)[len] = '\0';
		else
			break;
	}

	return len;
}

ssize_t read_ipmsg_fileinfo(int fd, void *buf, size_t count, size_t offset)
{
	ssize_t size;
	uint32_t headsize;

	if (offset < count)
		((char *)buf)[offset] = '\0';
	while (!offset || !strchr((char *)buf, ':')
		       || sscanf((char *)buf, "%" SCNx32, &headsize) != 1
		       || headsize > offset) {
		size = read(fd, (char *)buf + offset, count - offset);
		if (size == -1) {
			if (errno == EINTR)
				continue;
			pwarning(Fail, _("act: read data (%d),warning: %s\n"),
				 fd, strerror(errno));
			return -1;
		} else if (size == 0)
			return -1;
		if ((offset += size) < count)
			((char *)buf)[offset] = '\0';
		else
			break;
	}

	return offset;
}

FILE *Fopen(const char *path, const char *mode)
{
	FILE *stream;

	stream = fopen(path, mode);
	if (!stream)
		pwarning(Fail, _("act: revise text '%s',warning: %s\n"),
			 path, strerror(errno));

	return stream;
}

int Mkdir(const char *pathname, mode_t mode)
{
	int status;

	status = mkdir(pathname, mode);
	if (status != 0)
		pwarning(Fail, _("act: create directory '%s',warning: %s\n"),
			 pathname, strerror(errno));

	return status;
}

/////////
GtkWidget *create_box(gboolean vertical)
{
	GtkWidget *box;

	if (vertical)
		box = gtk_vbox_new(FALSE, 0);
	else
		box = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(box);

	return box;
}

GtkWidget *create_button_box(gboolean vertical)
{
	GtkWidget *bb;

	if (vertical)
		bb = gtk_vbutton_box_new();
	else
		bb = gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(bb), 1);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bb), GTK_BUTTONBOX_END);
	gtk_widget_show(bb);

	return bb;
}

GtkWidget *create_button(const gchar * title)
{
	GtkWidget *button;

	button = gtk_button_new_with_label(title);
	gtk_widget_show(button);

	return button;
}

GtkWidget *create_paned(gboolean vertical)
{
	GtkWidget *paned;

	if (vertical)
		paned = gtk_vpaned_new();
	else
		paned = gtk_hpaned_new();
	gtk_widget_show(paned);

	return paned;
}

GtkWidget *create_scrolled_window()
{
	GtkWidget *sw;

	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
					    GTK_SHADOW_ETCHED_IN);
	gtk_widget_show(sw);

	return sw;
}

GtkWidget *create_label(const gchar * title)
{
	GtkWidget *label;

	label = gtk_label_new(title);
	gtk_widget_show(label);

	return label;
}

GtkWidget *create_frame(const gchar * title)
{
	GtkWidget *frame;

	frame = gtk_frame_new(title);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
	gtk_widget_show(frame);

	return frame;
}

GtkWidget *create_text_view()
{
	GtkWidget *view;

	view = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD);
	gtk_widget_show(view);

	return view;
}

GtkWidget *create_window(const gchar * title, gdouble width, gdouble height)
{
	extern Control ctr;
	GtkWidget *window;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), title);
	gtk_window_set_default_size(GTK_WINDOW(window),
				    GINT(ctr.pix * width),
				    GINT(ctr.pix * height));
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_widget_show(window);

	return window;
}
