//
// C++ Implementation: TcpData
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "TcpData.h"
#include "CoreThread.h"
#include "MainWindow.h"
#include "SendFile.h"
#include "wrapper.h"
#include "utils.h"
extern CoreThread cthrd;
extern MainWindow mwin;

/**
 * 类构造函数.
 */
TcpData::TcpData():sock(-1), size(0)
{
}

/**
 * 类析构函数.
 */
TcpData::~TcpData()
{
        close(sock);
}

/**
 * TCP连接处理入口.
 * @param sock tcp socket
 */
void TcpData::TcpDataEntry(int sock)
{
        TcpData tdata;

        tdata.sock = sock;
        tdata.DispatchTcpData();
}

/**
 * 分派TCP数据处理方案.
 */
void TcpData::DispatchTcpData()
{
        uint32_t commandno;
        ssize_t len;

        /* 读取消息前缀 */
        if ((len = read_ipmsg_prefix(sock, buf, MAX_SOCKLEN)) <= 0)
                return;

        /* 分派消息 */
        size = len;     //设置缓冲区数据的有效长度
        commandno = iptux_get_dec_number(buf, ':', 4);  //获取命令字
        switch (GET_MODE(commandno)) {
        case IPMSG_GETFILEDATA:
                RequestData(IPMSG_FILE_REGULAR);
                break;
        case IPMSG_GETDIRFILES:
                RequestData(IPMSG_FILE_DIR);
                break;
        case IPTUX_SENDSUBLAYER:
                RecvSublayer(GET_OPT(commandno));
                break;
        default:
                break;
        }
}

/**
 * 请求文件(目录)数据.
 * @param fileattr 文件类型
 */
void TcpData::RequestData(uint32_t fileattr)
{
        SendFile sfile;
        const char *attachptr;
        char *attach;

        attachptr = iptux_skip_section(buf, ':', 5);
        switch (GET_MODE(fileattr)) {
                case IPMSG_FILE_REGULAR:
                        read_ipmsg_filedata(sock, (void *)attachptr,
                                            buf + MAX_SOCKLEN - attachptr,
                                            buf + size - attachptr);
                        break;
                case IPMSG_FILE_DIR:
                        read_ipmsg_dirfiles(sock, (void *)attachptr,
                                            buf + MAX_SOCKLEN - attachptr,
                                            buf + size - attachptr);
                        break;
                default:
                        break;
        }

        attach = ipmsg_get_attach(buf, ':', 5);
        sfile.RequestDataEntry(sock, fileattr, attach);
        g_free(attach);
}

/**
 * 接收底层数据.
 * @param cmdopt 命令字选项
 */
void TcpData::RecvSublayer(uint32_t cmdopt)
{
        static uint32_t count = 0;
        char path[MAX_PATHLEN];
        struct sockaddr_in addr;
        socklen_t len;
        PalInfo *pal;
        int fd;

        /* 检查好友是否存在 */
        len = sizeof(addr);
        getpeername(sock, (struct sockaddr *)&addr, &len);
        if (!(pal = cthrd.GetPalFromList(addr.sin_addr.s_addr)))
                return;

        /* 创建即将接收的数据文件路径 */
        switch (GET_OPT(cmdopt)) {
        case IPTUX_PHOTOPICOPT:
                snprintf(path, MAX_PATHLEN, "%s" PHOTO_PATH "/%" PRIx32,
                                 g_get_user_cache_dir(), pal->ipv4);
                break;
        case IPTUX_MSGPICOPT:
                snprintf(path, MAX_PATHLEN, "%s" PIC_PATH "/%" PRIx32
                         "-%" PRIx32 "-%lx", g_get_user_cache_dir(),
                         pal->ipv4, count++, time(NULL));
                break;
        default:
                snprintf(path, MAX_PATHLEN, "%s" IPTUX_PATH "/%" PRIx32
                         "-%" PRIx32 "-%lx", g_get_user_cache_dir(),
                         pal->ipv4, count++, time(NULL));
                break;
        }

        /* 终于可以接收数据了^_^ */
        if ((fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
                return;
        RecvSublayerData(fd, strlen(buf) + 1);
        close(fd);

        /* 分派数据 */
        switch (GET_OPT(cmdopt)) {
        case IPTUX_PHOTOPICOPT:
                RecvPhotoPic(pal, path);
                break;
        case IPTUX_MSGPICOPT:
                RecvMsgPic(pal, path);
                break;
        default:
                break;
        }
}

/**
 * 接收数据.
 * @param fd file descriptor
 * @param len 缓冲区无效数据长度
 */
void TcpData::RecvSublayerData(int fd, size_t len)
{
        ssize_t ssize;

        if (size != len)
                xwrite(fd, buf + len, size - len);
        do {
                if ((ssize = xread(sock, buf, MAX_SOCKLEN)) <= 0)
                        break;
                if ((ssize = xwrite(fd, buf, ssize)) <= 0)
                        break;
        } while (1);
}

/**
 * 接收好友形象照片.
 * @param pal class PalInfo
 * @param path file path
 */
void TcpData::RecvPhotoPic(PalInfo *pal, const char *path)
{
        g_free(pal->photo);
        pal->photo = g_strdup(path);
        gdk_threads_enter();
        pthread_mutex_lock(cthrd.GetMutex());
        cthrd.UpdatePalToList(pal->ipv4);
        pthread_mutex_unlock(cthrd.GetMutex());
        mwin.UpdateItemToPaltree(pal->ipv4);
        gdk_threads_leave();
}

/**
 * 接收消息图片.
 * @param pal class PalInfo
 * @param path file path
 */
void TcpData::RecvMsgPic(PalInfo *pal, const char *path)
{
        MsgPara para;
        ChipData *chip;

        /* 构建消息封装包 */
        para.pal = pal;
        para.stype = MESSAGE_SOURCE_TYPE_PAL;
        para.btype = GROUP_BELONG_TYPE_REGULAR;
        chip = new ChipData;
        chip->type = MESSAGE_CONTENT_TYPE_PICTURE;
        chip->data = g_strdup(path);
        para.dtlist = g_slist_append(NULL, chip);

        /* 交给某人处理吧 */
        cthrd.InsertMessage(&para);
}
