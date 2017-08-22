//
// C++ Implementation: Command
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Command.h"
#include "ProgramData.h"
#include "CoreThread.h"
#include "wrapper.h"
#include "support.h"
#include "utils.h"
extern ProgramData progdt;
extern CoreThread cthrd;
uint32_t Command::packetn = 1;

/**
 * 类构造函数.
 */
Command::Command():size(0)
{
}

/**
 * 类析构函数.
 */
Command::~Command()
{
}

/**
 * 向局域网所有计算机广播上线信息.
 * @param sock udp socket
 */
void Command::BroadCast(int sock)
{
        struct sockaddr_in addr;
        GSList *list, *tlist;

        CreateCommand(IPMSG_ABSENCEOPT | IPMSG_BR_ENTRY, progdt.nickname);
        ConvertEncode(progdt.encode);
        CreateIptuxExtra(progdt.encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        tlist = list = get_sys_broadcast_addr(sock);
        while (tlist) {
                addr.sin_addr.s_addr = GPOINTER_TO_UINT(tlist->data);
                sendto(sock, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));
                g_usleep(9999);
                tlist = g_slist_next(tlist);
        }
        g_slist_free(list);
}

/**
 * 向局域网某些计算机单独发送上线信息.
 * @param sock udp socket
 */
void Command::DialUp(int sock)
{
        struct sockaddr_in addr;
        in_addr_t startip, endip, ipv4;
        NetSegment *pns;
        GSList *list, *tlist;

        CreateCommand(IPMSG_DIALUPOPT | IPMSG_ABSENCEOPT | IPMSG_BR_ENTRY,
                                                         progdt.nickname);
        ConvertEncode(progdt.encode);
        CreateIptuxExtra(progdt.encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        //与某些代码片段的获取网段描述相冲突，必须复制出来使用
        pthread_mutex_lock(&progdt.mutex);
        list = progdt.CopyNetSegment();
        pthread_mutex_unlock(&progdt.mutex);
        tlist = list;
        while (tlist) {
                pns = (NetSegment *)tlist->data;
                inet_pton(AF_INET, pns->startip, &startip);
                startip = ntohl(startip);
                inet_pton(AF_INET, pns->endip, &endip);
                endip = ntohl(endip);
                ipv4_order(&startip, &endip);
                ipv4 = startip;
                while (ipv4 <= endip) {
                        addr.sin_addr.s_addr = htonl(ipv4);
                        sendto(sock, buf, size, 0, (struct sockaddr *)&addr,
                                                                 sizeof(addr));
                        g_usleep(999);
                        ipv4++;
                }
                tlist = g_slist_next(tlist);
        }
        for (tlist = list; tlist; tlist = g_slist_next(tlist))
                delete (NetSegment *)tlist->data;
        g_slist_free(list);
}

/**
 * 回复好友本人在线.
 * @param sock udp socket
 * @param pal class PalInfo
 */
void Command::SendAnsentry(int sock, PalInfo *pal)
{
        struct sockaddr_in addr;

        CreateCommand(IPMSG_ABSENCEOPT | IPMSG_ANSENTRY, progdt.nickname);
        ConvertEncode(pal->encode);
        CreateIptuxExtra(pal->encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = pal->ipv4;

        sendto(sock, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));
}

/**
 * 通告好友本人下线.
 * @param sock udp socket
 * @param pal class PalInfo
 */
void Command::SendExit(int sock, PalInfo *pal)
{
        struct sockaddr_in addr;

        CreateCommand(IPMSG_DIALUPOPT | IPMSG_BR_EXIT, NULL);
        ConvertEncode(pal->encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = pal->ipv4;

        sendto(sock, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));
}

/**
 * 通告好友本人个人信息已变.
 * @param sock udp socket
 * @param pal class PalInfo
 */
void Command::SendAbsence(int sock, PalInfo *pal)
{
        struct sockaddr_in addr;

        CreateCommand(IPMSG_ABSENCEOPT | IPMSG_BR_ABSENCE, progdt.nickname);
        ConvertEncode(pal->encode);
        CreateIptuxExtra(pal->encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = pal->ipv4;

        sendto(sock, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));
}

/**
 * 尝试着给某计算机发送一个上线信息数据包.
 * @param sock udp socket
 * @param ipv4 ipv4 address
 */
void Command::SendDetectPacket(int sock, in_addr_t ipv4)
{
        struct sockaddr_in addr;

        CreateCommand(IPMSG_DIALUPOPT | IPMSG_ABSENCEOPT | IPMSG_BR_ENTRY,
                                                         progdt.nickname);
        ConvertEncode(progdt.encode);
        CreateIptuxExtra(progdt.encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = ipv4;

        sendto(sock, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));
}

/**
 * 给好友发送消息.
 * @param sock udp socket
 * @param pal class PalInfo
 * @param msg 消息数据
 */
void Command::SendMessage(int sock, PalInfo *pal, const char *msg)
{
        struct sockaddr_in addr;
        uint32_t packetno;
        uint8_t count;

        pal->rpacketn = packetno = packetn;     //此数据包需要检验回复
        CreateCommand(IPMSG_SENDCHECKOPT | IPMSG_SENDMSG, msg);
        ConvertEncode(pal->encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = pal->ipv4;

        count = 0;
        do {
                sendto(sock, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));
                g_usleep(1000000);
                count++;
        } while (pal->rpacketn == packetno && count < MAX_RETRYTIMES);
        if (pal->rpacketn == packetno)
                FeedbackError(pal, GROUP_BELONG_TYPE_REGULAR,
                         _("Your pal didn't receive the packet. "
                           "He or she is offline maybe."));
}

/**
 * 回复已收到消息.
 * @param sock udp socket
 * @param pal class PalInfo
 * @param packetno 好友消息的包编号
 */
void Command::SendReply(int sock, PalInfo *pal, uint32_t packetno)
{
        char packetstr[11];     //10 +1 =11
        struct sockaddr_in addr;

        snprintf(packetstr, 11, "%" PRIu32, packetno);
        CreateCommand(IPMSG_SENDCHECKOPT | IPMSG_RECVMSG, packetstr);
        ConvertEncode(pal->encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = pal->ipv4;

        sendto(sock, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));
}

/**
 * 群发消息(被其他函数调用).
 * @param sock udp socket
 * @param pal class PalInfo
 * @param msg 消息数据
 */
void Command::SendGroupMsg(int sock, PalInfo *pal, const char *msg)
{
        struct sockaddr_in addr;

        CreateCommand(IPMSG_BROADCASTOPT | IPMSG_SENDMSG, msg);
        ConvertEncode(pal->encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = pal->ipv4;

        sendto(sock, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));
}

/**
 * 发送群组消息(被其他函数调用).
 * @param sock udp socket
 * @param pal class PalInfo
 * @param opttype 命令额外选项
 * @param msg 消息数据
 */
void Command::SendUnitMsg(int sock, PalInfo *pal, uint32_t opttype, const char *msg)
{
        struct sockaddr_in addr;

        CreateCommand(opttype | IPTUX_SENDMSG, msg);
        ConvertEncode(pal->encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = pal->ipv4;

        sendto(sock, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));
}

/**
 * 向好友请求文件数据.
 * @param sock tcp socket
 * @param pal class PalInfo
 * @param packetno 好友消息的包编号
 * @param fileid 文件ID标识
 * @param offset 文件偏移量
 * @return 消息发送成功与否
 */
bool Command::SendAskData(int sock, PalInfo *pal, uint32_t packetno,
                                 uint32_t fileid, int64_t offset)
{
        char attrstr[35];       //8+1+8+1+16 +1 =35
        struct sockaddr_in addr;
        const char *iptuxstr = "iptux";

        snprintf(attrstr, 35, "%" PRIx32 ":%" PRIx32 ":%" PRIx64,
                                         packetno, fileid, offset);
        //IPMSG和Feiq的命令字段都是只有IPMSG_GETFILEDATA,使用(IPMSG_FILEATTACHOPT | IPMSG_GETFILEDATA）
	//会产生一些潜在的不兼容问题,所以在发往非iptux时只使用IPMSG_GETFILEDATA
        if(strstr(pal->version,iptuxstr))
            CreateCommand(IPMSG_FILEATTACHOPT | IPMSG_GETFILEDATA, attrstr);
        else
            CreateCommand(IPMSG_GETFILEDATA, attrstr);
        ConvertEncode(pal->encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = pal->ipv4;

        if (((connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
                         && (errno != EINTR))
                 || (xwrite(sock, buf, size) == -1))
                return false;

        return true;
}

/**
 * 向好友请求目录文件.
 * @param sock tcp socket
 * @param pal class PalInfo
 * @param packetno 好友消息的包编号
 * @param fileid 文件ID标识
 * @return 消息发送成功与否
 */
bool Command::SendAskFiles(int sock, PalInfo *pal, uint32_t packetno, uint32_t fileid)
{
        char attrstr[20];       //8+1+8+1+1 +1  =20
        struct sockaddr_in addr;

        snprintf(attrstr, 20, "%" PRIx32 ":%" PRIx32 ":0", packetno, fileid);   //兼容LanQQ软件
        CreateCommand(IPMSG_FILEATTACHOPT | IPMSG_GETDIRFILES, attrstr);
        ConvertEncode(pal->encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = pal->ipv4;

        if (((connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
                         && (errno != EINTR))
                 || (xwrite(sock, buf, size) == -1))
                return false;

        return true;
}

/**
 * 向好友请求共享文件信息.
 * @param sock udp socket
 * @param pal class PalInfo
 * @param opttype 命令额外选项
 * @param attach 附加数据，即密码
 */
void Command::SendAskShared(int sock, PalInfo *pal, uint32_t opttype, const char *attach)
{
        struct sockaddr_in addr;

        CreateCommand(opttype | IPTUX_ASKSHARED, attach);
        ConvertEncode(pal->encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = pal->ipv4;

        sendto(sock, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));
}

/**
 * 向好友发送文件信息.
 * @param sock udp socket
 * @param pal class PalInfo
 * @param opttype 命令额外选项
 * @param extra 扩展数据，即文件信息
 */
void Command::SendFileInfo(int sock, PalInfo *pal, uint32_t opttype, const char *extra)
{
        struct sockaddr_in addr;

        CreateCommand(opttype | IPMSG_FILEATTACHOPT | IPMSG_SENDMSG, NULL);
        ConvertEncode(pal->encode);
        CreateIpmsgExtra(extra, pal->encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = pal->ipv4;

        sendto(sock, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));
}

/**
 * 发送本人的头像数据.
 * @param sock udp socket
 * @param pal class PalInfo
 */
void Command::SendMyIcon(int sock, PalInfo *pal)
{
        struct sockaddr_in addr;

        CreateCommand(IPTUX_SENDICON, NULL);
        ConvertEncode(pal->encode);
        CreateIconExtra();

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = pal->ipv4;

        sendto(sock, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));
}

/**
 * 发送本人的签名信息.
 * @param sock udp socket
 * @param pal class PalInfo
 */
void Command::SendMySign(int sock, PalInfo *pal)
{
        struct sockaddr_in addr;

        CreateCommand(IPTUX_SENDSIGN, progdt.sign);
        ConvertEncode(pal->encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = pal->ipv4;

        sendto(sock, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));
}

/**
 * 发送底层数据(即发送为最终用户所不能察觉的文件数据).
 * @param sock tcp socket
 * @param pal class PalInfo
 * @param opttype 命令额外选项
 * @param path 文件路径
 */
void Command::SendSublayer(int sock, PalInfo *pal, uint32_t opttype, const char *path)
{
        struct sockaddr_in addr;
        int fd;

        CreateCommand(opttype | IPTUX_SENDSUBLAYER, NULL);
        ConvertEncode(pal->encode);

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(IPTUX_DEFAULT_PORT);
        addr.sin_addr.s_addr = pal->ipv4;

        if (((connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
                         && (errno != EINTR))
                 || (xwrite(sock, buf, size) == -1)
                 || ((fd = open(path, O_RDONLY)) == -1))
                return;

        SendSublayerData(sock, fd);
        close(fd);
}

/**
 * 回馈错误消息.
 * @param pal class PalInfo
 * @param btype 消息归属类型
 * @param error 错误串
 */
void Command::FeedbackError(PalInfo *pal, GroupBelongType btype, const char *error)
{
        MsgPara para;
        ChipData *chip;

        /* 构建消息封装包 */
        para.pal = pal;
        para.stype = MESSAGE_SOURCE_TYPE_ERROR;
        para.btype = btype;
        chip = new ChipData;
        chip->type = MESSAGE_CONTENT_TYPE_STRING;
        chip->data = g_strdup(error);
        para.dtlist = g_slist_append(NULL, chip);

        /* 交给某人处理吧 */
        cthrd.InsertMessage(&para);
}

/**
 * 将文件描述符数据写入网络套接口.
 * @param sock tcp socket
 * @param fd file descriptor
 */
void Command::SendSublayerData(int sock, int fd)
{
        ssize_t len;

        do {
                if ((len = xread(fd, buf, MAX_UDPLEN)) <= 0)
                        break;
                if ((len = xwrite(sock, buf, len)) <= 0)
                        break;
        } while (1);
}

/**
 * 将缓冲区中的字符串转换为指定的编码.
 * @param encode 字符集编码
 */
void Command::ConvertEncode(const char *encode)
{
        char *ptr;

        if (encode && strcasecmp(encode, "utf-8") != 0
                 && (ptr = convert_encode(buf, encode, "utf-8"))) {
                size = strlen(ptr) + 1;
                memcpy(buf, ptr, size);
                g_free(ptr);
        }
}


/**
 * 创建命令数据.
 * @param command 命令字
 * @param attach 附加数据
 */
void Command::CreateCommand(uint32_t command, const char *attach)
{
        const gchar *env;
        char *ptr;

        snprintf(buf, MAX_UDPLEN, "%s", IPTUX_VERSION);
        size = strlen(buf);
        ptr = buf + size;

        snprintf(ptr, MAX_UDPLEN - size, ":%" PRIu32, packetn);
        packetn++;
        size += strlen(ptr);
        ptr = buf + size;

        env = g_get_user_name();
        snprintf(ptr, MAX_UDPLEN - size, ":%s", env);
        size += strlen(ptr);
        ptr = buf + size;

        env = g_get_host_name();
        snprintf(ptr, MAX_UDPLEN - size, ":%s", env);
        size += strlen(ptr);
        ptr = buf + size;

        if(command == IPMSG_GETFILEDATA)
            snprintf(ptr, MAX_UDPLEN - size, ":%d", command);
        else
            snprintf(ptr, MAX_UDPLEN - size, ":%" PRIu32, command);
        size += strlen(ptr);
        ptr = buf + size;

        snprintf(ptr, MAX_UDPLEN - size, ":%s", attach ? attach : "");
        size += strlen(ptr) + 1;
}

/**
 * 创建ipmsg的扩展数据(即文件信息).
 * @param extra 扩展数据
 * @param encode 字符集编码
 */
void Command::CreateIpmsgExtra(const char *extra, const char *encode)
{
        char *pptr, *ptr;

        pptr = buf + size;
        if (encode && strcasecmp(encode, "utf-8") != 0
                 && (ptr = convert_encode(extra, encode, "utf-8"))) {
                snprintf(pptr, MAX_UDPLEN - size, "%s", ptr);
                g_free(ptr);
        } else
                snprintf(pptr, MAX_UDPLEN - size, "%s", extra);
        if ( (ptr = strrchr(pptr, '\a')))
                *(ptr + 1) = '\0';
        size += strlen(pptr) + 1;
}

/**
 * 创建iptux程序独有的扩展数据.
 * @param encode 字符集编码
 */
void Command::CreateIptuxExtra(const char *encode)
{
        char *pptr, *ptr;

        pptr = buf + size;
        if (encode && strcasecmp(encode, "utf-8") != 0
                 && (ptr = convert_encode(progdt.mygroup, encode, "utf-8"))) {
                snprintf(pptr, MAX_UDPLEN - size, "%s", ptr);
                g_free(ptr);
        } else
                snprintf(pptr, MAX_UDPLEN - size, "%s", progdt.mygroup);
        size += strlen(pptr) + 1;

        pptr = buf + size;
        snprintf(pptr, MAX_UDPLEN - size, "%s", progdt.myicon);
        size += strlen(pptr) + 1;

        pptr = buf + size;
        snprintf(pptr, MAX_UDPLEN - size, "utf-8");
        size += strlen(pptr) + 1;
}

/**
 * 创建个人头像的扩展数据.
 */
void Command::CreateIconExtra()
{
        const gchar *env;
        char path[MAX_PATHLEN];
        ssize_t len;
        int fd;

        env = g_get_user_config_dir();
        snprintf(path, MAX_PATHLEN, "%s" ICON_PATH "/my-icon", env);
        if ((fd = open(path, O_RDONLY)) == -1)
                return;
        len = xread(fd, buf + size, MAX_UDPLEN - size);
        close(fd);
        if (len != -1)
                size += len;
}
