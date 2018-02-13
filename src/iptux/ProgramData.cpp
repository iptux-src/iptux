#include "ProgramData.h"

namespace iptux {

void ProgramData::InitSublayer() {
  ProgramDataCore::InitSublayer();
  CreateCursor();
  CreateTagTable();
}


/**
 * 创建鼠标光标.
 */
void ProgramData::CreateCursor() {
  xcursor = gdk_cursor_new(GDK_XTERM);
  lcursor = gdk_cursor_new(GDK_HAND2);
}

/**
 * 创建用于(text-view)的一些通用tag.
 * @note 给这些tag一个"global"标记，表示这些对象是全局共享的
 */
void ProgramData::CreateTagTable() {
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

ProgramData::ProgramData(IptuxConfig &config)
    : ProgramDataCore(config),
      xcursor(nullptr),
      lcursor(nullptr),
      table(nullptr) {
}
ProgramData::~ProgramData() {
  if(xcursor) {
    g_object_unref(xcursor);
  }
  if(lcursor) {
    g_object_unref(lcursor);
  }
  if (table) {
    g_object_unref(table);
  }
}

}