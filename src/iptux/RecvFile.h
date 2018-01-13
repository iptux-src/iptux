//
// C++ Interface: RecvFile
//
// Description:
// 接受相关的文件信息,不包含文件数据
//
// Author: cwll <cwll2009@126.com> ,(C) 2012.02
//        Jally <jallyx@163.com>, (C) 2008
// 2012.02:把文件接收确认和选择放在了聊天窗口，所以这个类的大部分功能都不用了
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_RECVFILE_H
#define IPTUX_RECVFILE_H

#include "iptux/mess.h"

namespace iptux {

class RecvFile {
public:
        RecvFile();
        ~RecvFile();
        static void RecvEntry(GData *para);
private:
        void ParseFilePara(GData **para);
        FileInfo *DivideFileinfo(char **extra);
};

}

#endif
