//
// C++ Implementation: SendFile
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "SendFile.h"
#include "SendFileData.h"
#include "CoreThread.h"
#include "Command.h"
#include "AnalogFS.h"
#include "utils.h"
extern CoreThread cthrd;

SendFile::SendFile()
{
}

SendFile::~SendFile()
{
}

/**
 * 发送本机共享文件信息入口.
 * @param pal class PalInfo
 */
void SendFile::SendSharedInfoEntry(PalInfo *pal)
{
        GSList *list;

        pthread_mutex_lock(cthrd.GetMutex());
        list = cthrd.GetPublicFileList();
        SendFileInfo(pal, IPTUX_SHAREDOPT, list);
        pthread_mutex_unlock(cthrd.GetMutex());
}

/**
 * 发送文件信息入口.
 * @param pal class PalInfo
 * @param flist 文件路径链表
 * @note 文件路径链表中的数据将被本函数处理掉
 */
void SendFile::SendFileInfoEntry(PalInfo *pal, GSList *flist)
{
        struct stat64 st;
        FileInfo *file;
        GSList *tlist, *list;

        /* 将文件路径链表转换为文件信息链表 */
        list = NULL;
        tlist = flist;
        while (tlist) {
                if (stat64((char *)tlist->data, &st) == -1
                         || !(S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))) {
                        g_free(tlist->data);
                        tlist = g_slist_next(tlist);
                }
                /* 加入文件信息到链表 */
                file = new FileInfo;
                list = g_slist_append(list, file);
                file->fileid = cthrd.PrnQuote()++;
                /* file->packetn = 0;//没必要设置此字段 */
                file->fileattr = S_ISREG(st.st_mode) ? IPMSG_FILE_REGULAR :
                                                         IPMSG_FILE_DIR;
                /* file->filesize = 0;//我喜欢延后处理 */
                /* file->fileown = NULL;//没必要设置此字段 */
                file->filepath = (char *)tlist->data;
                tlist = g_slist_next(tlist);
        }

        /* 发送文件信息 */
        SendFileInfo(pal, 0, list);

        /* 添加文件信息到中心点 */
        tlist = list;
        pthread_mutex_lock(cthrd.GetMutex());
        while (tlist) {
                cthrd.AttachFileToPrivate((FileInfo *)tlist->data);
                tlist = g_slist_next(tlist);
        }
        pthread_mutex_unlock(cthrd.GetMutex());
        g_slist_free(list);
}

/**
 * 广播文件信息入口.
 * @param plist 好友链表
 * @param flist 文件路径链表
 * @note 文件路径链表中的数据将被本函数处理掉
 */
void SendFile::BcstFileInfoEntry(GSList *plist, GSList *flist)
{
        struct stat64 st;
        FileInfo *file;
        GSList *tlist, *list;

        /* 将文件路径链表转换为文件信息链表 */
        list = NULL;
        tlist = flist;
        while (tlist) {
                if (stat64((char *)tlist->data, &st) == -1
                         || !(S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))) {
                        g_free(tlist->data);
                        tlist = g_slist_next(tlist);
                }
                /* 加入文件信息到链表 */
                file = new FileInfo;
                list = g_slist_append(list, file);
                file->fileid = cthrd.PrnQuote()++;
                /* file->packetn = 0;//没必要设置此字段 */
                file->fileattr = S_ISREG(st.st_mode) ? IPMSG_FILE_REGULAR :
                                                         IPMSG_FILE_DIR;
                /* file->filesize = 0;//我喜欢延后处理 */
                /* file->fileown = NULL;//没必要设置此字段 */
                file->filepath = (char *)tlist->data;
                tlist = g_slist_next(tlist);
        }

        /* 发送文件信息 */
        BcstFileInfo(plist, 0, list);

        /* 添加文件信息到中心点 */
        tlist = list;
        pthread_mutex_lock(cthrd.GetMutex());
        while (tlist) {
                cthrd.AttachFileToPrivate((FileInfo *)tlist->data);
                tlist = g_slist_next(tlist);
        }
        pthread_mutex_unlock(cthrd.GetMutex());
        g_slist_free(list);
}

/**
 * 请求文件数据入口.
 * @param sock tcp socket
 * @param fileattr 文件类型
 * @param attach 附加数据
 */
void SendFile::RequestDataEntry(int sock, uint32_t fileattr, char *attach)
{
        struct sockaddr_in addr;
        socklen_t len;
        PalInfo *pal;
        FileInfo *file, *nfile;
        uint32_t fileid;

        /* 检查文件属性是否匹配 */
        fileid = iptux_get_hex_number(attach, ':', 1);
        file = (FileInfo *)cthrd.GetFileFromAll(fileid);
        if (!file || GET_MODE(file->fileattr) != GET_MODE(fileattr))
                return;

        /* 检查好友数据是否存在 */
        len = sizeof(addr);
        getpeername(sock, (struct sockaddr *)&addr, &len);
        if (!(pal = cthrd.GetPalFromList(addr.sin_addr.s_addr)))
                return;

        /* 发送文件数据 */
        /**
         *文件信息可能被删除或修改，必须单独复制一份.
         */
        nfile = new FileInfo(*file);
        nfile->fileown = pal;
        nfile->filepath = g_strdup(file->filepath);
        ThreadSendFile(sock, nfile);
}

/**
 * 发送文件信息.
 * @param pal class PalInfo
 * @param opttype 命令字选项
 * @param filist 文件信息链表
 */
void SendFile::SendFileInfo(PalInfo *pal, uint32_t opttype, GSList *filist)
{
        AnalogFS afs;
        Command cmd;
        char buf[MAX_UDPLEN];
        size_t len;
        char *ptr, *name;
        GSList *tlist;
        FileInfo *file;

        /* 初始化 */
        len = 0;
        ptr = buf;
        buf[0] = '\0';

        /* 将文件信息写入缓冲区 */
        tlist = filist;
        while (tlist) {
                file = (FileInfo *)tlist->data;
                if (access(file->filepath, F_OK) == -1) {
                        tlist = g_slist_next(tlist);
                        continue;
                }
                name = ipmsg_get_filename_pal(file->filepath);  //获取面向好友的文件名
                file->filesize = afs.ftwsize(file->filepath);   //不得不计算文件长度了
                snprintf(ptr, MAX_UDPLEN - len, "%" PRIu32 ":%s:%" PRIx64 ":%"
                                 PRIx32 ":%" PRIx32 ":\a:", file->fileid, name,
                                 file->filesize, 0, file->fileattr);
                g_free(name);
                len += strlen(ptr);
                ptr = buf + len;
                tlist = g_slist_next(tlist);
        }

        /* 发送文件信息 */
        cmd.SendFileInfo(cthrd.UdpSockQuote(), pal, opttype, buf);
}

/**
 * 广播文件信息.
 * @param plist 好友链表
 * @param opttype 命令字选项
 * @param filist 文件信息链表
 */
void SendFile::BcstFileInfo(GSList *plist, uint32_t opttype, GSList *filist)
{
        AnalogFS afs;
        Command cmd;
        char buf[MAX_UDPLEN];
        size_t len;
        char *ptr, *name;
        GSList *tlist;
        FileInfo *file;

        /* 初始化 */
        len = 0;
        ptr = buf;
        buf[0] = '\0';

        /* 将文件信息写入缓冲区 */
        tlist = filist;
        while (tlist) {
                file = (FileInfo *)tlist->data;
                if (access(file->filepath, F_OK) == -1) {
                        tlist = g_slist_next(tlist);
                        continue;
                }
                name = ipmsg_get_filename_pal(file->filepath);  //获取面向好友的文件名
                file->filesize = afs.ftwsize(file->filepath);   //不得不计算文件长度了
                snprintf(ptr, MAX_UDPLEN - len, "%" PRIu32 ":%s:%" PRIx64 ":%"
                                 PRIx32 ":%" PRIx32 ":\a:", file->fileid, name,
                                 file->filesize, 0, file->fileattr);
                g_free(name);
                len += strlen(ptr);
                ptr = buf + len;
                tlist = g_slist_next(tlist);
        }

        /* 发送文件信息 */
        tlist = plist;
        while (tlist) {
                cmd.SendFileInfo(cthrd.UdpSockQuote(), (PalInfo *)tlist->data,
                                                                 opttype, buf);
                tlist = g_slist_next(tlist);
        }
}

/**
 * 发送文件数据.
 * @param sock tcp socket
 * @param file 文件信息
 */
void SendFile::ThreadSendFile(int sock, FileInfo *file)
{
        SendFileData sfdt(sock, file);

        sfdt.SendFileDataEntry();
        delete file;
}
