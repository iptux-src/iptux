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
 * @param flist 文件信息链表
 * @note 文件路径链表中的数据将被本函数处理掉
 */
void SendFile::SendFileInfoEntry(PalInfo *pal, GSList *flist)
{
        /* 发送文件信息 */
        SendFileInfo(pal, 0, flist);
}

/**
 * 广播文件信息入口.
 * @param plist 好友链表
 * @param flist 文件信息链表
 * @note 文件路径链表中的数据将被本函数处理掉
 */
void SendFile::BcstFileInfoEntry(GSList *plist, GSList *flist)
{
        BcstFileInfo(plist, 0, flist);
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
        FileInfo *file;
        uint32_t fileid;
        uint32_t filectime;
        /* 检查文件属性是否匹配 */

        fileid = iptux_get_hex_number(attach, ':', 1);
        file = (FileInfo *)cthrd.GetFileFromAll(fileid);
	/* 兼容windows版信鸽(IPMSG) ,这里的信鸽不是飞鸽传书(IPMSG)*/
	if(!file) {
                fileid = iptux_get_dec_number(attach, ':', 1);
		file = (FileInfo *)cthrd.GetFileFromAll(fileid);
	}
	/* 兼容adroid版信鸽(IPMSG) */
	if(!file) {
                fileid = iptux_get_hex_number(attach, ':', 0);
                filectime = iptux_get_dec_number(attach, ':', 1);
                file = (FileInfo *)cthrd.GetFileFromAllWithPacketN(fileid,filectime);
	}
        if (!file || GET_MODE(file->fileattr) != GET_MODE(fileattr))
                return;
        /* 检查好友数据是否存在 */
        len = sizeof(addr);
        getpeername(sock, (struct sockaddr *)&addr, &len);
        if (!(pal = cthrd.GetPalFromList(addr.sin_addr.s_addr)))
                return;

        /* 发送文件数据 */
//        /**
//         *文件信息可能被删除或修改，必须单独复制一份.
//         */
//        file->fileown = pal;
        ThreadSendFile(sock, file);
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
                file->packetn =  cmd.Packetn();
                snprintf(ptr, MAX_UDPLEN - len, "%" PRIu32 ":%s:%" PRIx64 ":%"
                                 PRIx32 ":%" PRIx32 ":\a", file->fileid, name,
                                 file->filesize, file->filectime, file->fileattr);
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
        GSList *tlist,*pallist,*filelist;
        FileInfo *file;

        /* 初始化 */
        len = 0;
        ptr = buf;
        buf[0] = '\0';
        pallist = plist;

        /* 将文件信息写入缓冲区 */
        while (pallist) {
            filelist = filist;
            while (filelist) {
                    file = (FileInfo *)filelist->data;
                    if(file->fileown == (PalInfo *)pallist->data) {
                        if (access(file->filepath, F_OK) == -1) {
                                filelist = g_slist_next(filelist);
                                continue;
                        }
                        name = ipmsg_get_filename_pal(file->filepath);  //获取面向好友的文件名
                        file->filesize = afs.ftwsize(file->filepath);   //不得不计算文件长度了
                        file->packetn =  cmd.Packetn();
                        snprintf(ptr, MAX_UDPLEN - len, "%" PRIu32 ":%s:%" PRIx64 ":%"
                                         PRIx32 ":%" PRIx32 ":\a:", file->fileid, name,
                                         file->filesize, file->filectime, file->fileattr);
                        g_free(name);
                        len += strlen(ptr);
                        ptr = buf + len;
                    }
                    filelist = g_slist_next(filelist);
            }
            cmd.SendFileInfo(cthrd.UdpSockQuote(), (PalInfo *)pallist->data,
                                                             opttype, buf);
            pallist = g_slist_next(pallist);
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
}
