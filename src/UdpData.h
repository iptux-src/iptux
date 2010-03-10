//
// C++ Interface: UdpData
//
// Description:
// 处理接收到的UDP数据
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef UDPDATA_H
#define UDPDATA_H

#include "mess.h"

class UdpData {
public:
        UdpData();
        ~UdpData();

        static void UdpDataEntry(in_addr_t ipv4, const char buf[], size_t size);
private:
        void DispatchUdpData();

        void SomeoneLost();
        void SomeoneEntry();
        void SomeoneExit();
        void SomeoneAnsentry();
        void SomeoneAbsence();
        void SomeoneSendmsg();
        void SomeoneRecvmsg();
        void SomeoneAskShared();
        void SomeoneSendIcon();
        void SomeoneSendSign();
        void SomeoneBcstmsg();

        PalInfo *CreatePalInfo();
        void UpdatePalInfo(PalInfo *pal);

        void InsertMessage(PalInfo *pal, GroupBelongType btype, const char *msg);
        void ConvertEncode(const char *enc);
        char *GetPalGroup();
        char *GetPalIcon();
        char *GetPalEncode();
        char *RecvPalIcon();
        PalInfo *AssertPalOnline();
        void RecvPalFile();

        in_addr_t ipv4;                 //数据来自
        size_t size;                    //缓冲区数据有效长度
        char buf[MAX_UDPLEN];   //数据缓冲区
        char *encode;                   //原数据编码(NULL意味着utf8)
private:
        static void ThreadAskSharedPasswd(PalInfo *pal);
        static void ThreadAskSharedFile(PalInfo *pal);
};

#endif
