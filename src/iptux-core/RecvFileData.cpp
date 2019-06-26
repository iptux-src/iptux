//
// C++ Implementation: RecvFileData
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
#include "RecvFileData.h"

#include <memory>

#include <fcntl.h>
#include <utime.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <glog/logging.h>

#include "iptux-core/Command.h"
#include "iptux-core/internal/AnalogFS.h"
#include "iptux-core/deplib.h"
#include "iptux-core/ipmsg.h"
#include "iptux-core/output.h"
#include "iptux-core/utils.h"
#include "iptux-core/Event.h"
#include "iptux-core/Exception.h"

using namespace std;

namespace iptux {

/**
 * 类构造函数.
 * @param fl 文件信息数据
 */
RecvFileData::RecvFileData(CoreThread* coreThread, FileInfo *fl)
    : coreThread(coreThread), file(fl), terminate(false), sumsize(0) {
  buf[0] = '\0';
  gettimeofday(&tasktime, NULL);
  /* gettimeofday(&filetime, NULL);//个人感觉没必要 */
}

/**
 * 类析构函数.
 */
RecvFileData::~RecvFileData() {}

/**
 * 接收文件数据入口.
 */
void RecvFileData::RecvFileDataEntry() {
  CHECK(GetTaskId() > 0);

  CreateUIPara();
  coreThread->emitEvent(make_shared<RecvFileStartedEvent>(GetTaskId()));
  /* 创建UI参考数据，并将数据主动加入UI */
  // gdk_threads_enter();
  // g_mwin->UpdateItemToTransTree(para);
  // auto g_progdt = g_cthrd->getUiProgramData();
  // if (g_progdt->IsAutoOpenFileTrans()) {
  //   g_mwin->OpenTransWindow();
  // }
  // gdk_threads_leave();

  /* 分类处理 */
  switch (file->fileattr) {
    case FileAttr::REGULAR:
      RecvRegularFile();
      break;
    case FileAttr::DIRECTORY:
      RecvDirFiles();
      break;
    default:
      break;
  }

  UpdateUIParaToOver();
  coreThread->emitEvent(make_shared<RecvFileFinishedEvent>(GetTaskId()));

  // /* 主动更新UI */
  // gdk_threads_enter();
  // g_mwin->UpdateItemToTransTree(para);
  // gdk_threads_leave();

  // /* 处理成功则播放提示音 */
  // if (!terminate && FLAG_ISSET(g_progdt->sndfgs, 2))
  //   g_sndsys->Playing(g_progdt->transtip);
}

/**
 * 获取UI参考数据.
 * @return UI参考数据
 */
const TransFileModel& RecvFileData::getTransFileModel() const { return para; }

/**
 * 终止过程处理.
 */
void RecvFileData::TerminateTrans() { terminate = true; }

/**
 * 创建UI参考数据.
 */
void RecvFileData::CreateUIPara() {
  struct in_addr addr = file->fileown->ipv4;
  para.setStatus("tip-recv")
      .setTask(_("receive"))
      .setPeer(file->fileown->name)
      .setIp(inet_ntoa(addr))
      .setFilename(ipmsg_get_filename_me(file->filepath, NULL))
      .setFileLength(file->filesize)
      .setFinishedLength(0)
      .setCost("00:00:00")
      .setRemain(_("Unknown"))
      .setRate("0 B/s")
      .setFilePath(file->filepath)
      .setTaskId(GetTaskId());
}

/**
 * 接收常规文件.
 */
void RecvFileData::RecvRegularFile() {
  AnalogFS afs;
  Command cmd(*coreThread);
  int64_t finishsize;
  int sock, fd;
  struct utimbuf timebuf;

  /* 创建文件传输套接口 */
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    LOG_ERROR(_("Fatal Error!!\nFailed to create new socket!\n%s"),
              strerror(errno));
    throw Exception(ErrorCode::CREATE_TCP_SOCKET_FAILED);
  }
  /* 请求文件数据 */
  if (!cmd.SendAskData(sock, file->fileown->GetKey(), file->packetn, file->fileid, 0)) {
    close(sock);
    terminate = true;  //标记处理过程失败
    return;
  }
  /* 打开文件 */
  if ((fd = afs.open(file->filepath, O_WRONLY | O_CREAT | O_TRUNC | O_LARGEFILE,
                     00644)) == -1) {
    close(sock);
    terminate = true;
    return;
  }

  /* 接收文件数据 */
  gettimeofday(&filetime, NULL);
  finishsize = RecvData(sock, fd, file->filesize, 0);
  close(fd);
  if (file->filectime != 0) {
    timebuf.actime = int(file->filectime);
    timebuf.modtime = int(file->filectime);
    utime(file->filepath, &timebuf);
  }
  //        sumsize += finishsize;

  /* 考察处理结果 */
  if (finishsize < file->filesize) {
    terminate = true;
    LOG_ERROR(_("Failed to receive the file \"%s\" from %s!"),
                       file->filepath, file->fileown->name);
  } else {
    LOG_INFO(_("Receive the file \"%s\" from %s successfully!"),
                       file->filepath, file->fileown->name);
  }
  /* 关闭文件传输套接口 */
  close(sock);
}

/**
 * 接收目录文件.
 */
void RecvFileData::RecvDirFiles() {
  AnalogFS afs;
  Command cmd(*coreThread);
  gchar *dirname, *pathname, *filename, *filectime, *filemtime;
  int64_t filesize, finishsize;
  uint32_t headsize, fileattr;
  int sock, fd;
  ssize_t size;
  size_t len;
  bool result;
  struct utimbuf timebuf;

  /* 创建文件传输套接口 */
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    LOG_ERROR(_("Fatal Error!!\nFailed to create new socket!\n%s"),
              strerror(errno));
    throw Exception(ErrorCode::CREATE_TCP_SOCKET_FAILED);
  }
  /* 请求目录文件 */
  if (!cmd.SendAskFiles(sock, file->fileown->GetKey(), file->packetn, file->fileid)) {
    close(sock);
    terminate = true;  //标记处理过程失败
    return;
  }
  /* 转到文件存档目录 */
  g_free(ipmsg_get_filename_me(file->filepath, &pathname));
  afs.mkdir(pathname, 0777);
  afs.chdir(pathname);
  g_free(pathname);

  /* 接收目录数据 */
  result = false;  //预设任务处理失败
  len = 0;         //预设缓冲区有效数据量为0
  while (!terminate) {
    /* 读取足够的数据，并分析数据头 */
    if ((size = read_ipmsg_fileinfo(sock, buf, MAX_SOCKLEN, len)) == -1) break;
    headsize = iptux_get_hex_number(buf, ':', 0);
    filename = ipmsg_get_filename(buf, ':', 1);
    filesize = iptux_get_hex64_number(buf, ':', 2);
    fileattr = iptux_get_hex_number(buf, ':', 3);
    filectime = iptux_get_section_string(buf, ':', 4);
    filemtime = iptux_get_section_string(buf, ':', 5);
    if (filectime != NULL)
      timebuf.actime = int(iptux_get_hex_number(filectime, '=', 1));
    if (filemtime != NULL)
      timebuf.modtime = int(iptux_get_hex_number(filemtime, '=', 1));
    len = size - headsize;  //更新缓冲区有效数据量

    /* 转码(如果好友不兼容iptux协议) */
    if (!file->fileown->isCompatible() &&
        strcasecmp(file->fileown->encode, "utf-8") != 0 &&
        (dirname = convert_encode(filename, "utf-8", file->fileown->encode)))
      g_free(filename);
    else
      dirname = filename;
    /* 更新UI参考值 */
    para.setFilename(dirname)
        .setFileLength(filesize)
        .setFinishedLength(0)
        .setCost("00:00:00")
        .setRemain(_("Unknown"))
        .setRate("0 B/s");

    /* 选择处理方案 */
    gettimeofday(&filetime, NULL);
    switch (GET_MODE(fileattr)) {
      case IPMSG_FILE_RETPARENT:
        afs.chdir("..");
        if (len) memmove(buf, buf + headsize, len);
        if (strlen(afs.cwd()) < strlen(file->filepath)) {
          //如果这时候还不成功结束就会陷入while开关第1句的死循环
          result = true;
          goto end;
        }
        continue;
      case IPMSG_FILE_DIR:
        afs.mkdir(dirname, 0777);
        afs.chdir(dirname);
        if (len) memmove(buf, buf + headsize, len);
        continue;
      case IPMSG_FILE_REGULAR:
        if ((fd = afs.open(dirname, O_WRONLY | O_CREAT | O_TRUNC | O_LARGEFILE,
                           00644)) == -1)
          goto end;
        break;
      default:
        if ((fd = open("/dev/null", O_WRONLY)) == -1) goto end;
        break;
    }

    /* 处理缓冲区剩余数据&读取文件数据 */
    size = int64_t(len) < filesize ? int64_t(len) : filesize;
    if (xwrite(fd, buf + headsize, size) == -1) {
      close(fd);
      goto end;
    }
    if (size == filesize) {  //文件数据读取已完成
      len -= size;
      if (len) memmove(buf, buf + headsize + size, len);
      finishsize = size;
    } else {    //尚需继续读取文件数据
      len = 0;  //首先标记缓冲区已无有效数据
      finishsize = RecvData(sock, fd, filesize, size);
      if (finishsize < filesize) {
        close(fd);
        goto end;
      }
    }
    close(fd);
    if (GET_MODE(fileattr) == IPMSG_FILE_REGULAR) {
      pathname = ipmsg_get_pathname_full(afs.cwd(), dirname);
      if (utime(pathname, &timebuf) < 0)
        g_print("Error to modify the file %s's filetime!\n", pathname);
      g_free(pathname);
    }
    //                sumsize += filesize;
  }
  result = true;

  /* 考察处理结果 */
end:
  if (!result) {
    terminate = true;
    LOG_ERROR(_("Failed to receive the directory \"%s\" from %s!"),
                       file->filepath, file->fileown->name);
  } else {
    LOG_INFO(_("Receive the directory \"%s\" from %s successfully!"),
                       file->filepath, file->fileown->name);
  }
  /* 关闭文件传输套接口 */
  close(sock);
}

/**
 * 接收文件数据.
 * @param sock tcp socket
 * @param fd file descriptor
 * @param filesize 文件总长度
 * @param offset 已读取数据量
 * @return 完成数据量
 */
int64_t RecvFileData::RecvData(int sock, int fd, int64_t filesize,
                               int64_t offset) {
  int64_t tmpsize, finishsize;
  struct timeval val1, val2;
  float difftime;
  uint32_t rate;
  ssize_t size;

  /* 如果文件数据已经完全被接收，则直接返回 */
  if (offset == filesize) return filesize;

  /* 接收数据 */
  tmpsize = finishsize = offset;  //初始化已读取数据量
  gettimeofday(&val1, NULL);      //初始化起始时间
  do {
    /* 接收数据并写入磁盘 */
    size = MAX_SOCKLEN < filesize - finishsize ? MAX_SOCKLEN
                                               : filesize - finishsize;
    if ((size = xread(sock, buf, size)) == -1) return finishsize;
    if (size > 0 && xwrite(fd, buf, size) == -1) return finishsize;
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
          .setRemain(numeric_to_time((uint32_t)((filesize - finishsize) / rate)))
          .setRate(numeric_to_rate(rate));
      val1 = val2;           //更新时间参考点
      tmpsize = finishsize;  //更新下载量
    }
  } while (!terminate && size && finishsize < filesize);

  return finishsize;
}

/**
 * 更新UI参考数据到任务结束.
 */
void RecvFileData::UpdateUIParaToOver() {
  struct timeval time;
  const char *statusfile;

  statusfile = terminate ? "tip-error" : "tip-finish";
  para.setStatus(statusfile);

  if (!terminate && file->fileattr == FileAttr::DIRECTORY) {
    para.setFilename(ipmsg_get_filename_me(file->filepath, NULL));
    para.setFileLength(sumsize);
    file->finishedsize = file->filesize;
  }
  if (!terminate) {
    gettimeofday(&time, NULL);
    para.setFinishedLength(para.getFileLength())
        .setCost(numeric_to_time((uint32_t)(difftimeval(time, tasktime))))
        .setRemain("")
        .setRate("");
    file->finishedsize = file->filesize;
  }
  para.finish();
}

}  // namespace iptux
