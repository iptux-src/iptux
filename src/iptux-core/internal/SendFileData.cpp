//
// C++ Implementation: SendFileData
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "config.h"
#include "SendFileData.h"

#include <cinttypes>
#include <memory>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <glib/gi18n.h>

#include "iptux-core/internal/AnalogFS.h"

#include "iptux-core/Event.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

/**
 * 类构造函数.
 * @param sk tcp socket
 * @param fl 文件信息数据
 */
SendFileData::SendFileData(CoreThread* coreThread, int sk, PFileInfo fl)
    : coreThread(coreThread), sock(sk), file(fl), terminate(false), sumsize(0) {
  buf[0] = '\0';
  gettimeofday(&tasktime, NULL);
}

/**
 * 类析构函数.
 */
SendFileData::~SendFileData() {}

/**
 * 发送文件数据入口.
 */
void SendFileData::SendFileDataEntry() {
  g_assert(GetTaskId() > 0);
  CreateUIPara();
  coreThread->emitEvent(make_shared<SendFileStartedEvent>(GetTaskId()));

  /* 分类处理 */
  switch (file->fileattr) {
    case FileAttr::REGULAR:
      SendRegularFile();
      break;
    case FileAttr::DIRECTORY:
      SendDirFiles();
      break;
    default:
      g_assert(false);
      break;
  }
  UpdateUIParaToOver();
  coreThread->emitEvent(make_shared<SendFileFinishedEvent>(GetTaskId()));
}

/**
 * 获取UI参考数据.
 * @return UI参考数据
 */
const TransFileModel& SendFileData::getTransFileModel() const {
  return para;
}

/**
 * 终止过程处理.
 */
void SendFileData::TerminateTrans() {
  terminate = true;
}

/**
 * 创建UI参考数据.
 */
void SendFileData::CreateUIPara() {
  struct in_addr addr = file->fileown->ipv4();

  para.setStatus("tip-send")
      .setTask(_("send"))
      .setPeer(file->fileown->getName())
      .setIp(inet_ntoa(addr))
      .setFilename(ipmsg_get_filename_me(file->filepath, NULL))
      .setFileLength(file->filesize)
      .setFinishedLength(0)
      .setCost("00:00:00")
      .setRemain(_("Unknown"))
      .setRate("0B/s")
      .setTaskId(GetTaskId());
}

/**
 * 发送常规文件.
 */
void SendFileData::SendRegularFile() {
  int64_t finishsize;
  int fd;

  /* 打开文件 */
  if ((fd = open(file->filepath, O_RDONLY | O_LARGEFILE)) == -1) {
    terminate = true;  // 标记处理过程失败
    return;
  }

  file->ensureFilesizeFilled();

  /* 发送文件数据 */
  gettimeofday(&filetime, NULL);
  finishsize = SendData(fd, file->filesize);
  close(fd);
  //        sumsize += finishsize;

  /* 考察处理结果 */
  if (finishsize < file->filesize) {
    terminate = true;
    LOG_INFO(_("Failed to send the file \"%s\" to %s!"), file->filepath,
             file->fileown->getName().c_str());
    // g_cthrd->SystemLog(_("Failed to send the file \"%s\" to %s!"),
    //                    file->filepath, file->fileown->name);
  } else {
    LOG_INFO(_("Send the file \"%s\" to %s successfully!"), file->filepath,
             file->fileown->getName().c_str());
    // g_cthrd->SystemLog(_("Send the file \"%s\" to %s successfully!"),
    //                    file->filepath, file->fileown->name);
  }
}

static void _dirstack_free(gpointer data, gpointer) {
  DIR* dir = (DIR*)data;
  if (dir) {
    closedir(dir);
  }
}

/**
 * 发送目录文件.
 */
void SendFileData::SendDirFiles() {
  AnalogFS afs;
  GQueue dirstack = G_QUEUE_INIT;  // @see DIR
  struct stat st;
  struct dirent *dirt, vdirt;
  DIR* dir;
  gchar *dirname, *pathname, *filename;
  int64_t finishsize;
  uint32_t headsize;
  int fd;
  bool result;

  /* 转到上传目录位置 */
  dirname = ipmsg_get_filename_me(file->filepath, &pathname);
  afs.chdir(pathname);
  g_free(pathname);
  strcpy(vdirt.d_name, dirname);
  dirt = &vdirt;
  g_free(dirname);

  result = false;  // 预设任务处理失败
  dir = NULL;      // 预设当前目录流无效
  goto start;
  while (!g_queue_is_empty(&dirstack)) {
    /* 取出最后一次压入堆栈的目录流 */
    dir = (DIR*)g_queue_pop_head(&dirstack);
    /* 发送目录流中的下属数据 */
    while (dir && (dirt = readdir(dir))) {
      if (strcmp(dirt->d_name, ".") == 0 || strcmp(dirt->d_name, "..") == 0)
        continue;

      /* 检查文件是否可用 */
    start:
      if (afs.stat(dirt->d_name, &st) == -1 ||
          !(S_ISREG(st.st_mode) || S_ISDIR(st.st_mode)))
        continue;
      /* 更新UI参考值 */
      para.setFilename(dirt->d_name)
          .setFileLength(st.st_size)
          .setFinishedLength(0)
          .setCost("00:00:00")
          .setRemain(_("Unknown"))
          .setRate("0B/s");
      /* 转码 */
      if (strcasecmp(file->fileown->getEncode().c_str(), "utf-8") != 0 &&
          (filename = convert_encode(
               dirt->d_name, file->fileown->getEncode().c_str(), "utf-8"))) {
        dirname = ipmsg_get_filename_pal(filename);
        g_free(filename);
      } else
        dirname = ipmsg_get_filename_pal(dirt->d_name);
      /* 构造数据头并发送 */
      snprintf(buf, MAX_SOCKLEN, "0000:%s:%.9jx:%lx:%lx=%jx:%lx=%jx:", dirname,
               (uintmax_t)(S_ISREG(st.st_mode) ? st.st_size : 0),
               S_ISREG(st.st_mode) ? IPMSG_FILE_REGULAR : IPMSG_FILE_DIR,
               IPMSG_FILE_MTIME, (uintmax_t)st.st_mtime, IPMSG_FILE_CREATETIME,
               (uintmax_t)st.st_ctime);
      g_free(dirname);
      headsize = strlen(buf);
      snprintf(buf, MAX_SOCKLEN, "%.4" PRIx32, headsize);
      *(buf + 4) = ':';
      if (xwrite(sock, buf, headsize) == -1)
        goto end;
      /* 选择处理方案 */
      gettimeofday(&filetime, NULL);
      if (S_ISREG(st.st_mode)) {  // 常规文件
        if ((fd = afs.open(dirt->d_name, O_RDONLY | O_LARGEFILE)) == -1)
          goto end;
        finishsize = SendData(fd, st.st_size);
        close(fd);
        if (finishsize < st.st_size)
          goto end;
        //                                sumsize += finishsize;
      } else if (S_ISDIR(st.st_mode)) {  // 目录文件
        if (dir)  // 若当前目录流有效则须压入堆栈
          g_queue_push_head(&dirstack, dir);
        /* 打开下属目录 */
        if (!(dir = afs.opendir(dirt->d_name)))
          goto end;
        /* 本地端也须转至下属目录 */
        afs.chdir(dirt->d_name);
      }
    }
    /* 目录流有效才可向上转 */
    if (dir) {
      /* 关闭当前操作的目录流 */
      closedir(dir);
      dir = NULL;
      /* 构造向上转的数据头并发送 */
      snprintf(buf, MAX_SOCKLEN,
               "0000:.:0:%lx:%lx=%jx:%lx=%jx:", IPMSG_FILE_RETPARENT,
               IPMSG_FILE_MTIME, (uintmax_t)st.st_mtime, IPMSG_FILE_CREATETIME,
               (uintmax_t)st.st_ctime);
      headsize = strlen(buf);
      snprintf(buf, MAX_SOCKLEN, "%.4" PRIx32, headsize);
      *(buf + 4) = ':';
      if (xwrite(sock, buf, headsize) == -1)
        goto end;
      /* 本地端也须向上转一层 */
      afs.chdir("..");
    }
  }
  result = true;

  /* 考察处理结果 */
end:
  if (!result) {
    /* 若当前目录流有效，则必须关闭 */
    if (dir)
      closedir(dir);
    /* 关闭堆栈中所有的目录流，并清空堆栈 */
    g_queue_foreach(&dirstack, _dirstack_free, NULL);
    g_queue_clear(&dirstack);
    LOG_INFO(_("Failed to send the directory \"%s\" to %s!"), file->filepath,
             file->fileown->getName().c_str());
    // g_cthrd->SystemLog(_("Failed to send the directory \"%s\" to %s!"),
    //                    file->filepath, file->fileown->name);
  } else {
    LOG_INFO(_("Send the directory \"%s\" to %s successfully!"), file->filepath,
             file->fileown->getName().c_str());
    // g_cthrd->SystemLog(_("Send the directory \"%s\" to %s successfully!"),
    //                    file->filepath, file->fileown->name);
  }
}

/**
 * 发送文件数据.
 * @param fd file descriptor
 * @param filesize 文件总长度
 * @return 完成数据量
 */
int64_t SendFileData::SendData(int fd, int64_t filesize) {
  int64_t tmpsize, finishsize;
  struct timeval val1, val2;
  float difftime;
  uint32_t rate;
  ssize_t size;

  /* 如果文件长度为0，则无须再进一步处理 */
  if (filesize == 0)
    return 0;

  tmpsize = finishsize = 0;   // 初始化已完成数据量
  gettimeofday(&val1, NULL);  // 初始化起始时间
  do {
    /* 读取文件数据并发送 */
    size = MAX_SOCKLEN < filesize - finishsize ? MAX_SOCKLEN
                                               : filesize - finishsize;
    if ((size = xread(fd, buf, MAX_SOCKLEN)) == -1)
      return finishsize;
    if (size > 0 && xwrite(sock, buf, size) == -1)
      return finishsize;
    finishsize += size;
    sumsize += size;
    file->finishedsize = sumsize;
    /* 判断是否需要更新UI参考值 */
    gettimeofday(&val2, NULL);
    difftime = difftimeval(val2, val1);
    if (difftime >= 1) {
      /* 更新UI参考值 */
      rate = (uint32_t)((finishsize - tmpsize) / difftime);
      para.setFinishedLength(finishsize)
          .setCost(numeric_to_time((uint32_t)(difftimeval(val2, filetime))))
          .setRemain(
              numeric_to_time((uint32_t)((filesize - finishsize) / rate)))
          .setRate(numeric_to_rate(rate));
      val1 = val2;           // 更新时间参考点
      tmpsize = finishsize;  // 更新下载量
    }
  } while (!terminate && size && finishsize < filesize);

  return finishsize;
}

/**
 * 更新UI参考数据到任务结束.
 */
void SendFileData::UpdateUIParaToOver() {
  struct timeval time;
  const char* statusfile;

  statusfile = terminate ? "tip-error" : "tip-finish";
  para.setStatus(statusfile);

  if (!terminate && file->fileattr == FileAttr::REGULAR) {
    para.setFilename(ipmsg_get_filename_me(file->filepath, NULL))
        .setFileLength(sumsize);
  }
  if (!terminate) {
    gettimeofday(&time, NULL);
    para.setFinishedLength(sumsize)
        .setCost(numeric_to_time((uint32_t)(difftimeval(time, tasktime))))
        .setRemain("")
        .setRate("");
  }
  para.finish();
}

}  // namespace iptux
