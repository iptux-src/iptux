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

#include "iptux-core/utils.h"

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

FileInfo * DivideFileinfo(char **extra);

/**
 * 文件接受入口.
 * @param para 文件参数
 */
void RecvFile::RecvEntry(CoreThread* coreThread,
  PPalInfo pal,
  const std::string extra,
  int packetno)
{
  auto extra2 = g_strdup(extra.c_str());
  auto extra3 = extra2;

  while (extra3 && *extra3) {
    auto file = iptux::DivideFileinfo(&extra3);
    file->packetn = packetno;
    file->fileown = pal;
    coreThread->emitEvent(make_shared<NewShareFileFromFriendEvent>(*file));
    delete file;
  }
  g_free(extra2);
}

/**
 * 从文件信息串中分离出文件信息数据.
 * @param extra 文件信息串
 * @return 文件信息数据
 */
FileInfo * DivideFileinfo(char **extra) {
  FileInfo *file;

  file = new FileInfo;
  file->fileid = iptux_get_dec_number(*extra, ':', 0);
  file->fileattr = iptux_get_hex_number(*extra, ':', 4);
  file->filesize = iptux_get_hex64_number(*extra, ':', 2);
  file->filepath = ipmsg_get_filename(*extra, ':', 1);
  file->filectime = iptux_get_hex_number(*extra, ':', 3);
  file->finishedsize = 0;

  //分割，格式1(\a) 格式2(:\a) 格式3(\a:) 格式4(:\a:)
  *extra = strchr(*extra, '\a');
  if (*extra)  //跳过'\a'字符
    (*extra)++;
  if (*extra && (**extra == ':'))  //跳过可能存在的':'字符
    (*extra)++;

  return file;
}

}  // namespace iptux
