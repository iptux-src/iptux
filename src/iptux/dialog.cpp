//
// C++ Implementation: dialog
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
#include "dialog.h"

#include "iptux-utils/utils.h"
#include <glib/gi18n.h>
#include <glog/logging.h>

#include "iptux/callback.h"

#include "iptux-utils/output.h"
#include "iptux/UiHelper.h"

using namespace std;

namespace iptux {

/**
 * 弹出请求程序退出的对话框.
 * @return true|false
 */
bool pop_request_quit(GtkWindow* parent) {
  GtkWidget* dialog;
  gint result;

  dialog =
      gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
                             GTK_BUTTONS_OK_CANCEL, "%s",
                             _("File transfer has not been completed.\n"
                               "Are you sure to cancel and quit?"));
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
  gtk_window_set_title(GTK_WINDOW(dialog), _("Confirm Exit"));

  result = gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);

  return (result == GTK_RESPONSE_OK);
}

/**
 * 弹出好友请求获取本机共享文件的对话框.
 * @param pal class PalInfo
 * @return true|false
 */
bool pop_request_shared_file(GtkWindow* parent, PalInfo* pal) {
  GtkWidget *dialog, *box;
  GtkWidget *label, *image;
  char* ptr;
  gint result;

  dialog = gtk_dialog_new_with_buttons(
      _("Request Shared Resources"), parent, GTK_DIALOG_MODAL, _("Agree"),
      GTK_RESPONSE_ACCEPT, _("Refuse"), GTK_RESPONSE_CANCEL, NULL);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                     box, TRUE, TRUE, 0);

  image = gtk_image_new_from_icon_name("dialog-question-symbolic",
                                       GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);
  image = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
  gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);

  string ipstr = inAddrToString(pal->ipv4());
  ptr = g_strdup_printf(_("Your pal (%s)[%s]\n"
                          "is requesting to get your shared resources,\n"
                          "Do you agree?"),
                        pal->getName().c_str(), ipstr.c_str());
  label = gtk_label_new(ptr);
  g_free(ptr);
  gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
  gtk_label_set_line_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
  gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 4);

  gtk_widget_show_all(dialog);
  result = gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);

  return (result == GTK_RESPONSE_ACCEPT);
}

/**
 * 弹出请求获取好友共享文件的密码.
 * @param pal class PalInfo
 * @return password string
 */
char* pop_obtain_shared_passwd(GtkWindow* parent, PalInfo* pal) {
  GtkWidget *dialog, *frame, *box;
  GtkWidget *image, *passwd;
  char* text;
  gint result;

  dialog = gtk_dialog_new_with_buttons(_("Access Password"), parent,
                                       GTK_DIALOG_MODAL, _("_OK"),
                                       GTK_RESPONSE_OK, NULL);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

  frame =
      gtk_frame_new(_("Please input the password for "
                      "the shared files behind"));
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                     frame, FALSE, FALSE, 0);
  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add(GTK_CONTAINER(frame), box);

  image = gtk_image_new_from_icon_name("dialog-password-symbolic",
                                       GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);
  image = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
  gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);
  string ipstr = inAddrToString(pal->ipv4());
  text = g_strdup_printf(_("(%s)[%s]Password:"), pal->getName().c_str(),
                         ipstr.c_str());
  frame = gtk_frame_new(text);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
  gtk_box_pack_start(GTK_BOX(box), frame, TRUE, TRUE, 0);
  g_free(text);
  passwd = gtk_entry_new();
  gtk_entry_set_activates_default(GTK_ENTRY(passwd), TRUE);
  gtk_entry_set_visibility(GTK_ENTRY(passwd), FALSE);
  gtk_container_add(GTK_CONTAINER(frame), passwd);

  gtk_widget_show_all(dialog);
  text = NULL;  // 并无多大用处，主要用来避免编译警告
mark:
  switch (result = gtk_dialog_run(GTK_DIALOG(dialog))) {
    case GTK_RESPONSE_OK:
      if (*(text = gtk_editable_get_chars(GTK_EDITABLE(passwd), 0, -1)) ==
          '\0') {
        gtk_widget_grab_focus(passwd);
        pop_warning(dialog, _("\nEmpty Password!"));
        g_free(text);
        goto mark;
      }
    default:
      break;
  }
  gtk_widget_destroy(dialog);

  return (result == GTK_RESPONSE_OK ? text : NULL);
}

/**
 * 弹出密码设定的对话框.
 * @param parent parent window
 * @return password string
 */
char* pop_password_settings(GtkWidget* parent) {
  CHECK_NOTNULL(parent);
  GtkWidget *dialog, *hbox, *passwd, *repeat;
  gchar *text1, *text2;
  gint result;

  dialog = gtk_dialog_new_with_buttons(
      _("Enter a New Password"), GTK_WINDOW(parent), GTK_DIALOG_MODAL, _("_OK"),
      GTK_RESPONSE_OK, _("_Cancel"), GTK_RESPONSE_CANCEL, NULL);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                     hbox, FALSE, FALSE, 0);
  passwd = gtk_label_new(_("Password: "));
  gtk_box_pack_start(GTK_BOX(hbox), passwd, FALSE, FALSE, 0);
  passwd = gtk_entry_new();
  gtk_entry_set_activates_default(GTK_ENTRY(passwd), TRUE);
  gtk_entry_set_visibility(GTK_ENTRY(passwd), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), passwd, TRUE, TRUE, 0);
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                     hbox, FALSE, FALSE, 0);
  repeat = gtk_label_new(_("Repeat: "));
  gtk_box_pack_start(GTK_BOX(hbox), repeat, FALSE, FALSE, 0);
  repeat = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(repeat), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), repeat, TRUE, TRUE, 0);

  gtk_widget_show_all(dialog);
  text1 = text2 = NULL;  // 并无多大用处，主要用来避免编译警告
mark:
  switch (result = gtk_dialog_run(GTK_DIALOG(dialog))) {
    case GTK_RESPONSE_OK:
      text1 = gtk_editable_get_chars(GTK_EDITABLE(passwd), 0, -1);
      text2 = gtk_editable_get_chars(GTK_EDITABLE(repeat), 0, -1);
      gtk_widget_grab_focus(passwd);
      if (strcmp(text1, text2) != 0) {
        pop_warning(dialog, _("\nPassword Mismatched!"));
        g_free(text1);
        g_free(text2);
        goto mark;
      } else if (*text1 == '\0') {
        pop_warning(dialog, _("\nEmpty Password!"));
        g_free(text1);
        g_free(text2);
        goto mark;
      }
    default:
      break;
  }
  gtk_widget_destroy(dialog);

  if (result == GTK_RESPONSE_OK) {
    g_free(text1);
    return text2;
  }
  return NULL;
}

const char* pop_save_path(GtkWidget* parent, const char* defaultPath) {
  const char* path = nullptr;
  GtkWidget* dialog;

  dialog = gtk_file_chooser_dialog_new(
      _("Please select a folder to save files."), GTK_WINDOW(parent),
      GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, _("_Cancel"), GTK_RESPONSE_CANCEL,
      _("_Save"), GTK_RESPONSE_ACCEPT, NULL);
  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), defaultPath);
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
  }
  gtk_widget_destroy(dialog);
  return path;
}

}  // namespace iptux
