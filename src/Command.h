//
// C++ Interface: Command
//
// Description:
// 创建命令并发送
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef COMMAND_H
#define COMMAND_H

#include "mess.h"

class Command {
public:
        Command();
        ~Command();

        void BroadCast(int sock);
        void DialUp(int sock);
        void SendAnsentry(int sock, PalInfo *pal);
        void SendExit(int sock, PalInfo *pal);
        void SendAbsence(int sock, PalInfo *pal);
        void SendDetectPacket(int sock, in_addr_t ipv4);
        void SendMessage(int sock, PalInfo *pal, const char *msg);
        void SendReply(int sock, PalInfo *pal, uint32_t packetno);
        void SendGroupMsg(int sock, PalInfo *pal, const char *msg);
        void SendUnitMsg(int sock, PalInfo *pal, uint32_t opttype, const char *msg);

        bool SendAskData(int sock, PalInfo *pal, uint32_t packetno,
                                 uint32_t fileid, int64_t offset);
        bool SendAskFiles(int sock, PalInfo *pal, uint32_t packetno, uint32_t fileid);
        void SendAskShared(int sock, PalInfo *pal, uint32_t opttype, const char *attach);
        void SendFileInfo(int sock, PalInfo *pal, uint32_t opttype, const char *extra);
        void SendMyIcon(int sock, PalInfo *pal);
        void SendMySign(int sock, PalInfo *pal);
        void SendSublayer(int sock, PalInfo *pal, uint32_t opttype, const char *path);
private:
        void FeedbackError(PalInfo *pal, GroupBelongType btype, const char *error);
        void SendSublayerData(int sock, int fd);
        void ConvertEncode(const char *encode);
        void CreateCommand(uint32_t command, const char *attach);
        void CreateIpmsgExtra(const char *extra, const char *encode);
        void CreateIptuxExtra(const char *encode);
        void CreateIconExtra();

        size_t size;                    //当前已使用缓冲区的长度
        char buf[MAX_UDPLEN];   //数据缓冲区
        static uint32_t packetn;                //包编号
public:
        inline uint32_t &Packetn() {
                        return packetn;
                }
};

#endif
