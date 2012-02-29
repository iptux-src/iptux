//
// C++ Implementation: SendFileData
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "SendFileData.h"
#include "ProgramData.h"
#include "MainWindow.h"
#include "LogSystem.h"
#include "SoundSystem.h"
#include "AnalogFS.h"
#include "wrapper.h"
#include "utils.h"
extern ProgramData progdt;
extern MainWindow mwin;
extern LogSystem lgsys;
extern SoundSystem sndsys;

/**
 * 类构造函数.
 * @param sk tcp socket
 * @param fl 文件信息数据
 */
SendFileData::SendFileData(int sk, FileInfo *fl):sock(sk), file(fl), para(NULL),
 terminate(false), sumsize(0)
{
        g_datalist_init(&para);
        gettimeofday(&tasktime, NULL);
        /* gettimeofday(&filetime, NULL);//个人感觉没必要 */
}

/**
 * 类析构函数.
 */
SendFileData::~SendFileData()
{
        g_datalist_clear(&para);
}

/**
 * 发送文件数据入口.
 */
void SendFileData::SendFileDataEntry()
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
                SendRegularFile();
                break;
        case IPMSG_FILE_DIR:
                SendDirFiles();
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
GData **SendFileData::GetTransFilePara()
{
        return &para;
}

/**
 * 终止过程处理.
 */
void SendFileData::TerminateTrans()
{
        terminate = true;
}

/**
 * 创建UI参考数据.
 */
void SendFileData::CreateUIPara()
{
        GtkIconTheme *theme;
        GdkPixbuf *pixbuf;
        struct in_addr addr;

        theme = gtk_icon_theme_get_default();
        if ((pixbuf = gtk_icon_theme_load_icon(theme, "tip-send",
                                               MAX_ICONSIZE,
                                               GtkIconLookupFlags(0), NULL)))
            g_datalist_set_data_full(&para, "status", pixbuf,
                                     GDestroyNotify(g_object_unref));
        g_datalist_set_data(&para, "task", (gpointer)(_("send")));
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
 * 发送常规文件.
 */
void SendFileData::SendRegularFile()
{
        int64_t finishsize;
        int fd;

        /* 打开文件 */
        if ((fd = open(file->filepath, O_RDONLY | O_LARGEFILE)) == -1) {
                terminate = true;       //标记处理过程失败
                return;
        }

        /* 发送文件数据 */
        gettimeofday(&filetime, NULL);
        finishsize = SendData(fd, file->filesize);
        close(fd);
//        sumsize += finishsize;

        /* 考察处理结果 */
        if (finishsize < file->filesize) {
                terminate = true;
                lgsys.SystemLog(_("Failed to send the file \"%s\" to %s!"),
                                         file->filepath, file->fileown->name);
        } else {
                lgsys.SystemLog(_("Send the file \"%s\" to %s successfully!"),
                                         file->filepath, file->fileown->name);
        }
}

/**
 * 发送目录文件.
 */
void SendFileData::SendDirFiles()
{
        AnalogFS afs;
        GQueue dirstack = G_QUEUE_INIT;
        struct stat64 st;
        struct dirent *dirt, vdirt;
        DIR *dir;
        gchar *dirname, *pathname, *filename;
        int64_t finishsize;
        uint32_t headsize;
        int fd;
        bool result;

        /* 转到上传目录位置 */
        dirname = ipmsg_get_filename_me(file->filepath, &pathname);
        afs.chdir(pathname);
        g_free(pathname);
        strcpy(vdirt.d_name, dirname);
        dirt = &vdirt;
        g_free(dirname);

        result = false; //预设任务处理失败
        dir = NULL;     //预设当前目录流无效
        goto start;
        while (!g_queue_is_empty(&dirstack)) {
                /* 取出最后一次压入堆栈的目录流 */
                dir = (DIR *)g_queue_pop_head(&dirstack);
                /* 发送目录流中的下属数据 */
                while (dir && (dirt = readdir(dir))) {
                        if (strcmp(dirt->d_name, ".") == 0
                            || strcmp(dirt->d_name, "..") == 0)
                                continue;

                        /* 检查文件是否可用 */
start:                  if (afs.stat(dirt->d_name, &st) == -1
                                 || !(S_ISREG(st.st_mode) || S_ISDIR(st.st_mode)))
                                continue;
                        /* 更新UI参考值 */
                        g_datalist_set_data_full(&para, "filename",
                                                 g_strdup(dirt->d_name),
                                                 GDestroyNotify(g_free));
                        g_datalist_set_data_full(&para, "filelength",
                                                 numeric_to_size(st.st_size),
                                                 GDestroyNotify(g_free));
                        g_datalist_set_data(&para, "finishlength", (gpointer)("0B"));
                        g_datalist_set_data(&para, "progress", GINT_TO_POINTER(0));
                        g_datalist_set_data(&para, "pro-text", (gpointer)("0.0%"));
                        g_datalist_set_data(&para, "cost", (gpointer)("00:00:00"));
                        g_datalist_set_data(&para, "remain", (gpointer)(_("unknown")));
                        g_datalist_set_data(&para, "rate", (gpointer)("0B/s"));
                        /* 转码 */
                        if (strcasecmp(file->fileown->encode, "utf-8") != 0
                                 && (filename = convert_encode(dirt->d_name,
                                         file->fileown->encode, "utf-8"))) {
                                dirname = ipmsg_get_filename_pal(filename);
                                g_free(filename);
                        } else
                                dirname = ipmsg_get_filename_pal(dirt->d_name);
                        /* 构造数据头并发送 */
                        snprintf(buf, MAX_SOCKLEN, "0000:%s:%.9" PRIx64 ":%lx:%lx=%lx:%lx=%lx:",
                                 dirname, S_ISREG(st.st_mode) ? st.st_size :0,
                                 S_ISREG(st.st_mode) ? IPMSG_FILE_REGULAR : IPMSG_FILE_DIR,
                                 IPMSG_FILE_MTIME,st.st_mtime,IPMSG_FILE_CREATETIME,st.st_ctime);
                        g_free(dirname);
                        headsize = strlen(buf);
                        snprintf(buf, MAX_SOCKLEN, "%.4" PRIx32, headsize);
                        *(buf + 4) = ':';
                        if (xwrite(sock, buf, headsize) == -1)
                                goto end;
                        /* 选择处理方案 */
                        gettimeofday(&filetime, NULL);
                        if (S_ISREG(st.st_mode)) {      //常规文件
                                if ((fd = afs.open(dirt->d_name, O_RDONLY |
                                                                 O_LARGEFILE)) == -1)
                                        goto end;
                                finishsize = SendData(fd, st.st_size);
                                close(fd);
                                if (finishsize < st.st_size)
                                        goto end;
//                                sumsize += finishsize;
                        } else if (S_ISDIR(st.st_mode)) {       //目录文件
                                if (dir)        //若当前目录流有效则须压入堆栈
                                        g_queue_push_head(&dirstack, dir);
                                /* 打开下属目录 */
                                if (!(dir = afs.opendir(dirt->d_name)))
                                        goto end;
                                /* 本地端也须转至下属目录 */
                                afs.chdir(dirt->d_name);
                        }
                }
                /* 目录流有效才可向上转 */
                if (dir) {
                        /* 关闭当前操作的目录流 */
                        closedir(dir);
                        dir = NULL;
                        /* 构造向上转的数据头并发送 */
                        snprintf(buf, MAX_SOCKLEN, "0000:.:0:%lx:%lx=%lx:%lx=%lx:", IPMSG_FILE_RETPARENT,
                                  IPMSG_FILE_MTIME,st.st_mtime,IPMSG_FILE_CREATETIME,st.st_ctime);
                        headsize = strlen(buf);
                        snprintf(buf, MAX_SOCKLEN, "%.4" PRIx32, headsize);
                        *(buf + 4) = ':';
                        if (xwrite(sock, buf, headsize) == -1)
                                goto end;
                        /* 本地端也须向上转一层 */
                        afs.chdir("..");
                }
        }
        result = true;

        /* 考察处理结果 */
end:    if (!result) {
                /* 若当前目录流有效，则必须关闭 */
                if (dir)
                        closedir(dir);
                /* 关闭堆栈中所有的目录流，并清空堆栈 */
                g_queue_foreach(&dirstack, GFunc(closedir), NULL);
                g_queue_clear(&dirstack);
                lgsys.SystemLog(_("Failed to send the directory \"%s\" to %s!"),
                                         file->filepath, file->fileown->name);
        } else {
                lgsys.SystemLog(_("Send the directory \"%s\" to %s successfully!"),
                                         file->filepath, file->fileown->name);
        }
}

/**
 * 发送文件数据.
 * @param fd file descriptor
 * @param filesize 文件总长度
 * @return 完成数据量
 */
int64_t SendFileData::SendData(int fd, int64_t filesize)
{
        int64_t tmpsize, finishsize;
        struct timeval val1, val2;
        float difftime, progress;
        uint32_t rate;
        ssize_t size;

        /* 如果文件长度为0，则无须再进一步处理 */
        if (filesize == 0)
                return 0;

        tmpsize = finishsize = 0;       //初始化已完成数据量
        gettimeofday(&val1, NULL);      //初始化起始时间
        do {
                /* 读取文件数据并发送 */
                size = MAX_SOCKLEN < filesize - finishsize ? MAX_SOCKLEN :
                                                 filesize - finishsize;
                if ((size = xread(fd, buf, MAX_SOCKLEN)) == -1)
                        return finishsize;
                if (size > 0 && xwrite(sock, buf, size) == -1)
                        return finishsize;
                finishsize += size;
                sumsize += size;
                file->finishedsize = sumsize;
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
void SendFileData::UpdateUIParaToOver()
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
