//
// C++ Implementation: RecvFileData
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "RecvFileData.h"
#include "ProgramData.h"
#include "MainWindow.h"
#include "LogSystem.h"
#include "SoundSystem.h"
#include "AnalogFS.h"
#include "Command.h"
#include "wrapper.h"
#include "output.h"
#include "utils.h"
#include <utime.h>

extern ProgramData progdt;
extern MainWindow mwin;
extern LogSystem lgsys;
extern SoundSystem sndsys;

/**
 * 类构造函数.
 * @param fl 文件信息数据
 */
RecvFileData::RecvFileData(FileInfo *fl):file(fl), para(NULL), terminate(false),
 sumsize(0)
{
        g_datalist_init(&para);
        gettimeofday(&tasktime, NULL);
        /* gettimeofday(&filetime, NULL);//个人感觉没必要 */
}

/**
 * 类析构函数.
 */
RecvFileData::~RecvFileData()
{
        g_datalist_clear(&para);
}

/**
 * 接收文件数据入口.
 */
void RecvFileData::RecvFileDataEntry()
{
        /* 创建UI参考数据，并将数据主动加入UI */
        gdk_threads_enter();
        CreateUIPara();
        mwin.UpdateItemToTransTree(&para);
        if (FLAG_ISSET(progdt.flags, 5))
                mwin.OpenTransWindow();
        gdk_threads_leave();

        /* 分类处理 */
        switch (GET_MODE(file->fileattr)) {
        case IPMSG_FILE_REGULAR:
                RecvRegularFile();
                break;
        case IPMSG_FILE_DIR:
                RecvDirFiles();
                break;
        default:
                break;
        }

        /* 主动更新UI */
        gdk_threads_enter();
        UpdateUIParaToOver();
        mwin.UpdateItemToTransTree(&para);
        gdk_threads_leave();

        /* 处理成功则播放提示音 */
        if (!terminate && FLAG_ISSET(progdt.sndfgs, 2))
                sndsys.Playing(progdt.transtip);
}

/**
 * 获取UI参考数据.
 * @return UI参考数据
 */
GData **RecvFileData::GetTransFilePara()
{
        return &para;
}

/**
 * 终止过程处理.
 */
void RecvFileData::TerminateTrans()
{
        terminate = true;
}

/**
 * 创建UI参考数据.
 */
void RecvFileData::CreateUIPara()
{
        GtkIconTheme *theme;
        GdkPixbuf *pixbuf;
        struct in_addr addr;

        theme = gtk_icon_theme_get_default();
        if ( (pixbuf = gtk_icon_theme_load_icon(theme, "tip-recv", MAX_ICONSIZE,
                                                GtkIconLookupFlags(0), NULL)))
            g_datalist_set_data_full(&para, "status", pixbuf,
                                     GDestroyNotify(g_object_unref));
        
        g_datalist_set_data(&para, "task", (gpointer)(_("receive")));
        g_datalist_set_data_full(&para, "peer", g_strdup(file->fileown->name),
                                                 GDestroyNotify(g_free));
        addr.s_addr = file->fileown->ipv4;
        g_datalist_set_data_full(&para, "ip", g_strdup(inet_ntoa(addr)),
                                 GDestroyNotify(g_free));
        g_datalist_set_data_full(&para, "filename",
                                 ipmsg_get_filename_me(file->filepath, NULL),
                                 GDestroyNotify(g_free));
        g_datalist_set_data_full(&para, "filelength",
                                 numeric_to_size(file->filesize),
                                 GDestroyNotify(g_free));
        g_datalist_set_data(&para, "finishlength", (gpointer)("0B"));
        g_datalist_set_data(&para, "progress", GINT_TO_POINTER(0));
        g_datalist_set_data(&para, "pro-text", (gpointer)("0.0%"));
        g_datalist_set_data(&para, "cost", (gpointer)("00:00:00"));
        g_datalist_set_data(&para, "remain", (gpointer)(_("unknown")));
        g_datalist_set_data(&para, "rate", (gpointer)("0B/s"));
        g_datalist_set_data(&para, "data", this);
}

/**
 * 接收常规文件.
 */
void RecvFileData::RecvRegularFile()
{
        AnalogFS afs;
        Command cmd;
        int64_t finishsize;
        int sock, fd;
        struct utimbuf timebuf;

        /* 创建文件传输套接口 */
        if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
                pop_error(_("Fatal Error!!\nFailed to create new socket!\n%s"),
                                                         strerror(errno));
                exit(1);
        }
        /* 请求文件数据 */
        if (!cmd.SendAskData(sock, file->fileown, file->packetn, file->fileid, 0)) {
                close(sock);
                terminate = true;       //标记处理过程失败
                return;
        }
        /* 打开文件 */
        if ((fd = afs.open(file->filepath, O_WRONLY | O_CREAT |
                                         O_TRUNC | O_LARGEFILE, 00644)) == -1) {
                close(sock);
                terminate = true;
                return;
        }

        /* 接收文件数据 */
        gettimeofday(&filetime, NULL);
        finishsize = RecvData(sock, fd, file->filesize, 0);
        close(fd);
        if(file->filectime != 0) {
            timebuf.actime = int(file->filectime);
            timebuf.modtime = int(file->filectime);
            utime(file->filepath,&timebuf);
        }
        sumsize += finishsize;

        /* 考察处理结果 */
        if (finishsize < file->filesize) {
                terminate = true;
                lgsys.SystemLog(_("Failed to receive the file \"%s\" from %s!"),
                                         file->filepath, file->fileown->name);
        } else {
                lgsys.SystemLog(_("Receive the file \"%s\" from %s successfully!"),
                                                 file->filepath, file->fileown->name);
        }
        /* 关闭文件传输套接口 */
        close(sock);
}

/**
 * 接收目录文件.
 */
void RecvFileData::RecvDirFiles()
{
        AnalogFS afs;
        Command cmd;
        gchar *dirname, *pathname, *filename, *filectime,*filemtime;
        int64_t filesize, finishsize;
        uint32_t headsize, fileattr;
        int sock, fd;
        ssize_t size;
        size_t len;
        bool result;
        struct utimbuf timebuf;

        /* 创建文件传输套接口 */
        if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
                pop_error(_("Fatal Error!!\nFailed to create new socket!\n%s"),
                                                         strerror(errno));
                exit(1);
        }
        /* 请求目录文件 */
        if (!cmd.SendAskFiles(sock, file->fileown, file->packetn, file->fileid)) {
                close(sock);
                terminate = true;       //标记处理过程失败
                return;
        }
        /* 转到文件存档目录 */
        g_free(ipmsg_get_filename_me(file->filepath, &pathname));
        afs.mkdir(pathname, 0777);
        afs.chdir(pathname);
        g_free(pathname);

        /* 接收目录数据 */
        result = false; //预设任务处理失败
        len = 0;        //预设缓冲区有效数据量为0
        while (!terminate) {
                /* 读取足够的数据，并分析数据头 */
                if ((size = read_ipmsg_fileinfo(sock, buf, MAX_SOCKLEN, len)) == -1)
                        break;
                headsize = iptux_get_hex_number(buf, ':', 0);
                filename = ipmsg_get_filename(buf, ':', 1);
                filesize = iptux_get_hex64_number(buf, ':', 2);
                fileattr = iptux_get_hex_number(buf, ':', 3);
                filectime = iptux_get_section_string(buf, ':', 4);
                filemtime = iptux_get_section_string(buf, ':', 5);
                if (filectime != NULL)
                    timebuf.actime = int(iptux_get_hex_number(filectime, '=', 1));
                if (filemtime != NULL)
                    timebuf.modtime = int(iptux_get_hex_number(filemtime, '=', 1));
                len = size - headsize;  //更新缓冲区有效数据量

                /* 转码(如果好友不兼容iptux协议) */
                if (!FLAG_ISSET(file->fileown->flags, 0)
                         && strcasecmp(file->fileown->encode, "utf-8") != 0
                         && (dirname = convert_encode(filename, "utf-8",
                                                 file->fileown->encode)))
                        g_free(filename);
                else
                        dirname = filename;
                /* 更新UI参考值 */
                //要有谁敢在下一段代码中释放(dirname)，那可别怪我没提醒哦
                g_datalist_set_data_full(&para, "filename", dirname,
                                         GDestroyNotify(g_free));
                g_datalist_set_data_full(&para, "filelength",numeric_to_size(filesize),
                                                         GDestroyNotify(g_free));
                g_datalist_set_data(&para, "finishlength", (gpointer)("0B"));
                g_datalist_set_data(&para, "progress", GINT_TO_POINTER(0));
                g_datalist_set_data(&para, "pro-text", (gpointer)("0.0%"));
                g_datalist_set_data(&para, "cost", (gpointer)("00:00:00"));
                g_datalist_set_data(&para, "remain", (gpointer)(_("unknown")));
                g_datalist_set_data(&para, "rate", (gpointer)("0B/s"));

                /* 选择处理方案 */
                gettimeofday(&filetime, NULL);
                switch (GET_MODE(fileattr)) {
                case IPMSG_FILE_RETPARENT:
                        afs.chdir("..");
                        if (len)
                                memmove(buf, buf + headsize, len);
                        if( strlen(afs.cwd()) < strlen(file->filepath))
                        {
                            //如果这时候还不成功结束就会陷入while开关第1句的死循环
                            result = true;
                            goto end;
                        }
                        continue;
                case IPMSG_FILE_DIR:
                        afs.mkdir(dirname, 0777);
                        afs.chdir(dirname);
                        if (len)
                                memmove(buf, buf + headsize, len);
                        continue;
                case IPMSG_FILE_REGULAR:
                        if ((fd = afs.open(dirname, O_WRONLY | O_CREAT |
                                                 O_TRUNC | O_LARGEFILE, 00644)) == -1)
                                goto end;
                        break;
                default:
                        if ((fd = open("/dev/null", O_WRONLY)) == -1)
                                goto end;
                        break;
                }

                /* 处理缓冲区剩余数据&读取文件数据 */
                size = len < filesize ? len : filesize;
                if (xwrite(fd, buf + headsize, size) == -1) {
                        close(fd);
                        goto end;
                }
                if (size == filesize) { //文件数据读取已完成
                        len -= size;
                        if (len)
                                memmove(buf, buf + headsize + size, len);
                        finishsize = size;
                } else {        //尚需继续读取文件数据
                        len = 0;        //首先标记缓冲区已无有效数据
                        finishsize = RecvData(sock, fd, filesize, size);
                        if (finishsize < filesize) {
                                close(fd);
                                goto end;
                        }
                }
                close(fd);
                if(GET_MODE(fileattr) == IPMSG_FILE_REGULAR) {
                    pathname = ipmsg_get_pathname_full(afs.cwd(),dirname);
                    if(utime(pathname,&timebuf) < 0)
                        g_print("Error to modify the file %s's filetime!\n",pathname);
                    g_free(pathname);
                }
                sumsize += filesize;
        }
        result = true;

        /* 考察处理结果 */
end:    if (!result) {
                terminate = true;
                lgsys.SystemLog(_("Failed to receive the directory \"%s\" from %s!"),
                                                 file->filepath, file->fileown->name);
        } else {
                lgsys.SystemLog(_("Receive the directory \"%s\" from %s successfully!"),
                                                 file->filepath, file->fileown->name);
        }
        /* 关闭文件传输套接口 */
        close(sock);
}

/**
 * 接收文件数据.
 * @param sock tcp socket
 * @param fd file descriptor
 * @param filesize 文件总长度
 * @param offset 已读取数据量
 * @return 完成数据量
 */
int64_t RecvFileData::RecvData(int sock, int fd, int64_t filesize, int64_t offset)
{
        int64_t tmpsize, finishsize;
        struct timeval val1, val2;
        float difftime, progress;
        uint32_t rate;
        ssize_t size;

        /* 如果文件数据已经完全被接收，则直接返回 */
        if (offset == filesize)
                return filesize;

        /* 接收数据 */
        tmpsize = finishsize = offset;  //初始化已读取数据量
        gettimeofday(&val1, NULL);      //初始化起始时间
        do {
                /* 接收数据并写入磁盘 */
                size = MAX_SOCKLEN < filesize - finishsize ? MAX_SOCKLEN :
                                                 filesize - finishsize;
                if ((size = xread(sock, buf, size)) == -1)
                        return finishsize;
                if (size > 0 && xwrite(fd, buf, size) == -1)
                        return finishsize;
                finishsize += size;
                /* 判断是否需要更新UI参考值 */
                gettimeofday(&val2, NULL);
                difftime = difftimeval(val2, val1);
                if (difftime >= 1) {
                        /* 更新UI参考值 */
                        progress = percent(finishsize, filesize);
                        rate = (uint32_t)((finishsize - tmpsize) / difftime);
                        g_datalist_set_data_full(&para, "finishlength",
                                         numeric_to_size(finishsize),
                                         GDestroyNotify(g_free));
                        g_datalist_set_data(&para, "progress",
                                         GINT_TO_POINTER(GINT(progress)));
                        g_datalist_set_data_full(&para, "pro-text",
                                         g_strdup_printf("%.1f", progress),
                                         GDestroyNotify(g_free));
                        g_datalist_set_data_full(&para, "cost", numeric_to_time(
                                         (uint32_t)(difftimeval(val2, filetime))),
                                         GDestroyNotify(g_free));
                        g_datalist_set_data_full(&para, "remain", numeric_to_time(
                                         (uint32_t)((filesize - finishsize) / rate)),
                                         GDestroyNotify(g_free));
                        g_datalist_set_data_full(&para, "rate",
                                         numeric_to_rate(rate),
                                         GDestroyNotify(g_free));
                        val1 = val2;    //更新时间参考点
                        tmpsize = finishsize;   //更新下载量
                }
        } while (!terminate && size && finishsize < filesize);

        return finishsize;
}

/**
 * 更新UI参考数据到任务结束.
 */
void RecvFileData::UpdateUIParaToOver()
{
        GtkIconTheme *theme;
        GdkPixbuf *pixbuf;
        struct timeval time;
        const char *statusfile;

        theme = gtk_icon_theme_get_default();
        statusfile = terminate ? "tip-error" : "tip-finish";
        if ( (pixbuf = gtk_icon_theme_load_icon(theme, statusfile, MAX_ICONSIZE,
                                                 GtkIconLookupFlags(0), NULL)))
                g_datalist_set_data_full(&para, "status", pixbuf,
                                 GDestroyNotify(g_object_unref));

        if (!terminate && GET_MODE(file->fileattr) == IPMSG_FILE_DIR) {
                g_datalist_set_data_full(&para, "filename",
                                 ipmsg_get_filename_me(file->filepath, NULL),
                                 GDestroyNotify(g_free));
                g_datalist_set_data_full(&para, "filelength", numeric_to_size(sumsize),
                                                         GDestroyNotify(g_free));
        }
        if (!terminate) {
                gettimeofday(&time, NULL);
                g_datalist_set_data_full(&para, "finishlength", numeric_to_size(sumsize),
                                                         GDestroyNotify(g_free));
                g_datalist_set_data(&para, "progress", GINT_TO_POINTER(100));
                g_datalist_set_data(&para, "pro-text", (gpointer)("100%"));
                g_datalist_set_data_full(&para, "cost", numeric_to_time(
                                         (uint32_t)(difftimeval(time, tasktime))),
                                         GDestroyNotify(g_free));
                g_datalist_set_data(&para, "remain", NULL);
                g_datalist_set_data(&para, "rate", NULL);
        }
        g_datalist_set_data(&para, "data", NULL);
}
