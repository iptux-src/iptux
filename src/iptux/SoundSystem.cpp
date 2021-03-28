//
// C++ Implementation: SoundSystem
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
#include "SoundSystem.h"

#ifdef GST_FOUND
#include <cstring>
#include <glib/gi18n.h>
#include <sys/time.h>

#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"
#include "iptux/UiProgramData.h"
#include "iptux/global.h"

using namespace std;

namespace iptux {
/**
 * 类构造函数.
 */
SoundSystem::SoundSystem() : eltset(NULL), persist(false) {
  g_datalist_init(&eltset);
  gettimeofday(&timestamp, NULL);
}

/**
 * 类析构函数.
 */
SoundSystem::~SoundSystem() {
  GstElement* pipeline;

  if ((pipeline =
           GST_ELEMENT(g_datalist_get_data(&eltset, "pipeline-element"))))
    gst_element_set_state(pipeline, GST_STATE_NULL);
  g_datalist_clear(&eltset);
}

/**
 * 初始化声音系统.
 */
void SoundSystem::InitSublayer() {
  GstElement* pipeline;
  GstElement *filesrc, *decode, *volume, *convert, *sink;
  GstBus* bus;

  gst_init(NULL, NULL);
  pipeline = gst_pipeline_new("sound-system");
  g_datalist_set_data_full(&eltset, "pipeline-element", pipeline,
                           GDestroyNotify(gst_object_unref));
  filesrc = gst_element_factory_make("filesrc", "source");
  g_datalist_set_data(&eltset, "filesrc-element", filesrc);
  decode = gst_element_factory_make("decodebin", "decode");
  g_datalist_set_data(&eltset, "decode-element", decode);
  volume = gst_element_factory_make("volume", "volume");
  g_datalist_set_data(&eltset, "volume-element", volume);
  convert = gst_element_factory_make("audioconvert", "convert");
  g_datalist_set_data(&eltset, "convert-element", convert);
  sink = gst_element_factory_make("autoaudiosink", "output");
  g_datalist_set_data(&eltset, "output-element", sink);

  if (!filesrc || !decode || !volume || !convert || !sink) {
    LOG_WARN("init sound system failed");
    return;
  }

  gst_bin_add_many(GST_BIN(pipeline), filesrc, decode, volume, convert, sink,
                   NULL);
  if (!gst_element_link_many(filesrc, decode, NULL)) {
    LOG_WARN("init sound system failed");
    return;
  }
  if (!gst_element_link_many(volume, convert, sink, NULL)) {
    LOG_WARN("init sound system failed");
    return;
  }
  g_signal_connect_swapped(decode, "pad-added", G_CALLBACK(LinkElement),
                           &eltset);

  bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  gst_bus_add_signal_watch(GST_BUS(bus));
  g_signal_connect_swapped(bus, "message::error", G_CALLBACK(ErrorMessageOccur),
                           this);
  g_signal_connect_swapped(bus, "message::eos", G_CALLBACK(EosMessageOccur),
                           this);
  gst_object_unref(bus);

  g_object_set(volume, "volume", g_cthrd->getProgramData()->volume, NULL);
}

/**
 * 调整音量.
 * @param value 音量值
 */
void SoundSystem::AdjustVolume(double value) {
  GstElement* volume;

  volume = GST_ELEMENT(g_datalist_get_data(&eltset, "volume-element"));
  g_object_set(volume, "volume", value, NULL);
}

/**
 * 播放音频文件.
 * @param file 音频文件
 * @note 如果时间间隔过短，系统将会忽略后一个请求.
 */
void SoundSystem::Playing(const char* file) {
  GstElement *pipeline, *filesrc;
  struct timeval time;

  gettimeofday(&time, NULL);
  if (!FLAG_ISSET(g_cthrd->getProgramData()->sndfgs, 0) ||
      (difftimeval(time, timestamp) < 0.1))
    return;

  if (persist)
    EosMessageOccur(this);
  persist = true;
  filesrc = GST_ELEMENT(g_datalist_get_data(&eltset, "filesrc-element"));
  g_object_set(filesrc, "location", file, NULL);
  pipeline = GST_ELEMENT(g_datalist_get_data(&eltset, "pipeline-element"));
  auto res = gst_element_set_state(pipeline, GST_STATE_PLAYING);
  if (res == GST_STATE_CHANGE_FAILURE) {
    LOG_WARN("gst_element_set_state failed: %d", res);
  }
  timestamp = time;
}

/**
 * 停止播放.
 */
void SoundSystem::Stop() {
  EosMessageOccur(this);
}

/**
 * 链接元素.
 */
void SoundSystem::LinkElement(GData** eltset, GstPad* pad) {
  GstElement* volume;
  GstCaps* caps;
  GstStructure* str;
  GstPad* spad;

  caps = gst_pad_query_caps(pad, NULL);
  str = gst_caps_get_structure(caps, 0);
  volume = GST_ELEMENT(g_datalist_get_data(eltset, "volume-element"));
  if (strcasestr(gst_structure_get_name(str), "audio") &&
      (spad = gst_element_get_compatible_pad(volume, pad, caps))) {
    auto ret = gst_pad_link(pad, spad);
    if (ret != GST_PAD_LINK_OK) {
      LOG_WARN("gst_pad_link failed: %d", ret);
    }
  }
  gst_caps_unref(caps);
}

/**
 * 错误响应处理函数.
 */
void SoundSystem::ErrorMessageOccur(SoundSystem* sndsys, GstMessage* message) {
  GstElement* pipeline;
  GError* error;
  gchar* dbgInfo = nullptr;

  gst_message_parse_error(message, &error, &dbgInfo);
  LOG_WARN(_("Failed to play the prompt tone, [%s] %s, %s"),
           GST_MESSAGE_SRC_NAME(message), error->message, dbgInfo);
  g_error_free(error);
  g_free(dbgInfo);
  EosMessageOccur(sndsys);
  pipeline =
      GST_ELEMENT(g_datalist_get_data(&sndsys->eltset, "pipeline-element"));
  gst_element_set_state(pipeline, GST_STATE_NULL);
}

/**
 * 播放文件结束响应处理函数.
 */
void SoundSystem::EosMessageOccur(SoundSystem* sndsys) {
  GstElement *pipeline, *decode, *volume;

  pipeline =
      GST_ELEMENT(g_datalist_get_data(&sndsys->eltset, "pipeline-element"));
  gst_element_set_state(pipeline, GST_STATE_READY);
  decode = GST_ELEMENT(g_datalist_get_data(&sndsys->eltset, "decode-element"));
  volume = GST_ELEMENT(g_datalist_get_data(&sndsys->eltset, "volume-element"));
  gst_element_unlink(decode, volume);
  sndsys->persist = false;
}

}  // namespace iptux

#else
namespace iptux {

SoundSystem::SoundSystem() {}
SoundSystem::~SoundSystem() {}
void SoundSystem::InitSublayer() {}
void SoundSystem::AdjustVolume(double value) {}
void SoundSystem::Playing(const char* file) {}
void SoundSystem::Stop() {}

}  // namespace iptux
#endif
