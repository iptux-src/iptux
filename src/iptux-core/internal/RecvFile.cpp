//
// C++ Implementation: RecvFile
//
// Description:
//
//
// Author: cwll <cwll2009@126.com> ,(C) 2012.02
//        Jally <jallyx@163.com>, (C) 2008
// 2012.02:把文件接收确认和选择放在了聊天窗口，所以这个类的大部分功能都不用了
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "config.h"
#include "RecvFile.h"

#include <cstring>

#include "iptux-core/Exception.h"
#include "iptux-core/internal/Command.h"
#include "iptux-core/internal/TransAbstract.h"
#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

/**
 * 类构造函数.
 */
RecvFile::RecvFile() {}

/**
 * 类析构函数.
 */
RecvFile::~RecvFile() {}

FileInfo* DivideFileinfo(char** extra);

/**
 * 文件接受入口.
 * @param para 文件参数
 */
void RecvFile::RecvEntry(CoreThread* coreThread,
                         PPalInfo pal,
                         const std::string extra,
                         int packetno) {
  auto fileInfos = Command::decodeFileInfos(extra);
  for (auto fileInfo : fileInfos) {
    fileInfo.packetn = packetno;
    fileInfo.fileown = pal;
    coreThread->emitEvent(make_shared<NewShareFileFromFriendEvent>(pal->GetKey(), fileInfo));
  }
}

}  // namespace iptux
