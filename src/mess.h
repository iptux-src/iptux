//
// C++ Interface: mess
//
// Description:
// 很杂乱的一些数据基本结构.
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef UDT_H
#define UDT_H

#include "sys.h"
#include "net.h"
#include "deplib.h"
#include "ipmsg.h"

/***************偶是可爱的分割线(枚举变量)*****************/

/**
 * 消息来源类型.
 */
typedef enum {
        MESSAGE_SOURCE_TYPE_PAL,        ///< 好友
        MESSAGE_SOURCE_TYPE_SELF,       ///< 自身
        MESSAGE_SOURCE_TYPE_ERROR       ///< 错误
}MessageSourceType;

/**
 * 消息内容类型.
 */
typedef enum {
        MESSAGE_CONTENT_TYPE_STRING,    ///< 字符串
        MESSAGE_CONTENT_TYPE_PICTURE    ///< 图片
}MessageContentType;

/**
 * 群组所属类型
 */
typedef enum {
        GROUP_BELONG_TYPE_REGULAR,      ///< 常规
        GROUP_BELONG_TYPE_SEGMENT,      ///< 分段
        GROUP_BELONG_TYPE_GROUP,        ///< 分组
        GROUP_BELONG_TYPE_BROADCAST     ///< 广播
}GroupBelongType;

/***************偶是可爱的分割线(数据对象)*****************/

/**
 * 好友信息.
 * flags位含义: \n
 * 黑名单(:3);此IP地址被列入黑名单(deprecated) \n
 * 更改(:2);好友的信息被用户手工修改，程序不应再更改好友的信息 \n
 * 在线(:1);好友依旧在线 \n
 * 兼容(:0);完全兼容iptux，程序将采用扩展协议与好友通信 \n
 */
class PalInfo {
public:
        PalInfo();
        ~PalInfo();

        in_addr_t ipv4; ///< 好友IP
        char *segdes;   ///< 所在网段描述
        char *version;  ///< 版本串 *
        char *user;     ///< 好友用户 *
        char *host;     ///< 好友主机 *
        char *name;     ///< 昵称 *
        char *group;    ///< 所在群组
        char *photo;    ///< 形象照片
        char *sign;     ///< 个性签名
        char *iconfile; ///< 好友头像 *
        char *encode;   ///< 好友编码 *
        uint8_t flags;  ///< 3 黑名单:2 更改:1 在线:0 兼容

        uint32_t packetn;       ///< 已接受最大的包编号
        uint32_t rpacketn;      ///< 需要接受检查的包编号
};

/**
 * 群组信息.
 */
class GroupInfo {
public:
        GroupInfo();
        ~GroupInfo();

        GQuark grpid;   ///< 唯一标识
        GroupBelongType type;   ///< 群组类型
        char *name;     ///< 群组名称 *
        GSList *member; ///< 群组成员(数据不为本链表拥有)
        GtkTextBuffer *buffer;  ///< 消息缓冲区 *
        GtkWidget *dialog;      ///< 对话框(若存在则必须与对话框类关联)
};

/**
 * 文件信息.
 */
class FileInfo {
public:
        FileInfo();
        ~FileInfo();

        uint32_t fileid;        ///< 唯一标识
        uint32_t packetn;       ///< 包编号
        uint32_t fileattr;      ///< 文件属性
        int64_t filesize;       ///< 文件大小
        int64_t finishedsize;   ///< 已完成大小
        PalInfo *fileown;       ///< 文件拥有者(来自好友*)
        char *filepath; ///< 文件路径 *
        uint32_t filectime;        ///<  文件创建时间
        uint32_t filemtime;        ///<  文件最后修改时间
        uint32_t filenum;          ///<  包内编号
};

/**
 * 消息参数.
 */
class MsgPara {
public:
        MsgPara();
        ~MsgPara();

        PalInfo *pal;   ///< 好友数据信息(来自好友*)
        MessageSourceType stype;        ///< 来源类型
        GroupBelongType btype;  ///< 所属类型
        GSList *dtlist; ///< 数据链表 *
};

/**
 * 碎片数据.
 */
class ChipData {
public:
        ChipData();
        ~ChipData();

        MessageContentType type;        ///< 消息内容类型
        char *data;     ///< 数据串 *
};

/**
 * 网段数据.
 */
class NetSegment {
public:
        NetSegment();
        ~NetSegment();

        char *startip;  ///< IP起始地址 *
        char *endip;    ///< IP终止地址 *
        char *description;      ///< 此IP段描述
};

/***************偶是可爱的分割线(抽象类)*****************/

/**
 * 会话抽象类.
 * 提供好友会话类必需的公共接口.
 */
class SessionAbstract {
public:
        SessionAbstract();
        virtual ~SessionAbstract();

        virtual void UpdatePalData(PalInfo *pal) = 0;   ///< 更新好友数据
        virtual void InsertPalData(PalInfo *pal) = 0;   ///< 插入好友数据
        virtual void DelPalData(PalInfo *pal) = 0;      ///< 删除好友数据
        virtual void ClearAllPalData() = 0;             ///< 清除所有好友数据
//        virtual void ShowEnclosure() = 0;               ///< 显示附件
        virtual void AttachEnclosure(const GSList *list) = 0;   ///< 添加附件
        virtual void OnNewMessageComing() = 0;          ///< 窗口打开情况下有新消息
};

/**
 * 传输抽象类.
 * 提供文件传输类必需的公共接口.
 */
class TransAbstract {
public:
        TransAbstract();
        virtual ~TransAbstract();

        /**
         * GData数据如下: \n
         * [GdkPixbuf]status, [gchar]task, [gchar]peer, [gchar]filename, [gchar]filelength,
         * [gchar]finishlength, [gint]progress, [gchar]pro-text, [gchar]cost, [gchar]remain,
         * [gchar]rate, [gpointer]data \n
         * @see MainWindow::CreateTransModel()
         */
        virtual GData **GetTransFilePara() = 0; ///< 获取更新UI的数据
        virtual void TerminateTrans() = 0;      ///< 终止过程处理
};

#endif
