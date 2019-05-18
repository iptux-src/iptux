#include "config.h"
#include "UiProgramData.h"

#include <unistd.h>
#include <sys/time.h>

#include "iptux/deplib.h"
#include "iptux/ipmsg.h"
#include "iptux/utils.h"

using namespace std;

namespace iptux {

void UiProgramData::InitSublayer() {
  CheckIconTheme();
  CreateTagTable();
}

/**
 * 创建用于(text-view)的一些通用tag.
 * @note 给这些tag一个"global"标记，表示这些对象是全局共享的
 */
void UiProgramData::CreateTagTable() {
  GtkTextTag *tag;

  table = gtk_text_tag_table_new();

  tag = gtk_text_tag_new("pal-color");
  g_object_set(tag, "foreground", "blue", NULL);
  g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
  gtk_text_tag_table_add(table, tag);
  g_object_unref(tag);

  tag = gtk_text_tag_new("me-color");
  g_object_set(tag, "foreground", "green", NULL);
  g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
  gtk_text_tag_table_add(table, tag);
  g_object_unref(tag);

  tag = gtk_text_tag_new("error-color");
  g_object_set(tag, "foreground", "red", NULL);
  g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
  gtk_text_tag_table_add(table, tag);
  g_object_unref(tag);

  tag = gtk_text_tag_new("sign-words");
  g_object_set(tag, "indent", 10, "foreground", "#1005F0", "font",
               "Sans Italic 8", NULL);
  g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
  gtk_text_tag_table_add(table, tag);
  g_object_unref(tag);

  tag = gtk_text_tag_new("url-link");
  g_object_set(tag, "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE,
               NULL);
  g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
  gtk_text_tag_table_add(table, tag);
  g_object_unref(tag);
}

/**
 * 确保头像数据被存放在主题库中.
 */
void UiProgramData::CheckIconTheme() {
  char pathbuf[MAX_PATHLEN];
  GdkPixbuf *pixbuf;

  snprintf(pathbuf, MAX_PATHLEN, __PIXMAPS_PATH "/icon/%s", myicon.c_str());
  if (access(pathbuf, F_OK) != 0) {
    snprintf(pathbuf, MAX_PATHLEN, "%s" ICON_PATH "/%s",
             g_get_user_config_dir(), myicon.c_str());
    if ((pixbuf = gdk_pixbuf_new_from_file(pathbuf, NULL))) {
      gtk_icon_theme_add_builtin_icon(myicon.c_str(), MAX_ICONSIZE, pixbuf);
      g_object_unref(pixbuf);
    }
  }

  snprintf(pathbuf, MAX_PATHLEN, __PIXMAPS_PATH "/icon/%s", palicon);
  if (access(pathbuf, F_OK) != 0) {
    snprintf(pathbuf, MAX_PATHLEN, "%s" ICON_PATH "/%s",
             g_get_user_config_dir(), palicon);
    if ((pixbuf = gdk_pixbuf_new_from_file(pathbuf, NULL))) {
      gtk_icon_theme_add_builtin_icon(palicon, MAX_ICONSIZE, pixbuf);
      g_object_unref(pixbuf);
    }
  }
}


UiProgramData::UiProgramData(shared_ptr<IptuxConfig> config)
    : ProgramData(config),
      table(nullptr) {
  InitSublayer();
}

UiProgramData::~UiProgramData() {
  if (table) {
    g_object_unref(table);
  }
}

bool UiProgramData::IsMsgSoundEnabled() const {
  return FLAG_ISSET(sndfgs, 1);
}

}
