//
// C++ Implementation: callback
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "callback.h"
#include "ProgramData.h"
#include "StatusIcon.h"
#include "MainWindow.h"
#include "support.h"
extern ProgramData progdt;
extern StatusIcon sicon;
extern MainWindow mwin;

/**
 * 改变UI的外观.
 * @return Gtk+库所需
 */
gboolean alter_interface_mode()
{
        sicon.AlterStatusIconMode();
        if(sicon.IsEmbedded())
            mwin.AlterWindowMode();
        else
            gtk_main_quit();

        return TRUE;
}

/**
 * 给entry控件设置提示信息.
 * @param entry entry
 * @param x the x coordinate of the cursor position
 * @param y the y coordinate of the cursor position
 * @param key TRUE if the tooltip was trigged using the keyboard
 * @param tooltip a GtkTooltip
 * @param text text string
 * @return Gtk+库所需
 */
gboolean entry_query_tooltip(GtkWidget *entry, gint x, gint y,
                         gboolean key, GtkTooltip *tooltip, char *text)
{
        GtkWidget *label;

        label = gtk_label_new(text);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_label_set_line_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
        gtk_tooltip_set_custom(tooltip, label);

        return TRUE;
}

/**
 * 保证entry控件只接收数字字符.
 * @param entry entry
 * @param text the new text to insert
 * @param length the length of the new text, in bytes, or -1 if new_text is nul-terminated
 */
void entry_insert_numeric(GtkWidget *entry, gchar *text, gint length)
{
        gint count;

        if (length == -1)
                length = strlen(text);
        count = 0;
        while (count < length) {
                if (!isdigit(*(text + count)) && !(*(text + count) == '.')) {
                        g_signal_stop_emission_by_name(entry, "insert-text");
                        return;
                }
                count++;
        }
}

/**
 * 以可预览的方式选择文件.
 * @param title file chooser dialog title
 * @param parent parent
 * @return 文件名
 */
gchar *choose_file_with_preview(const gchar *title, GtkWidget *parent)
{
        GtkWidget *chooser, *preview;
        gchar *filename;

        chooser = gtk_file_chooser_dialog_new(title, GTK_WINDOW(parent),
                         GTK_FILE_CHOOSER_ACTION_OPEN,
                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(chooser), GTK_RESPONSE_ACCEPT);
        gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(chooser), TRUE);
        gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(chooser), FALSE);
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(chooser), TRUE);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), g_get_home_dir());

        preview = gtk_image_new();
        gtk_widget_set_size_request(preview, MAX_PREVIEWSIZE, MAX_PREVIEWSIZE);
        gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(chooser), preview);
        gtk_file_chooser_set_preview_widget_active(GTK_FILE_CHOOSER(chooser), FALSE);
        g_signal_connect(chooser, "update-preview",
                         G_CALLBACK(chooser_update_preview), preview);

        gtk_widget_show_all(chooser);
        filename = NULL;
        switch (gtk_dialog_run(GTK_DIALOG(chooser))) {
        case GTK_RESPONSE_ACCEPT:
                filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
                break;
        default:
                break;
        }
        gtk_widget_destroy(chooser);

        return filename;
}

/**
 * 更新文件选择器的预览控件.
 * @param chooser a file chooser
 * @param preview preview widget
 */
void chooser_update_preview(GtkFileChooser *chooser, GtkWidget *preview)
{
        gchar *filename;
        GdkPixbuf *pixbuf;

        if (!(filename = gtk_file_chooser_get_preview_filename(chooser))) {
                gtk_file_chooser_set_preview_widget_active(chooser, FALSE);
                return;
        }

        pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
        g_free(filename);
        if (!pixbuf) {
                gtk_file_chooser_set_preview_widget_active(chooser, FALSE);
                return;
        }

        pixbuf_shrink_scale_1(&pixbuf, MAX_PREVIEWSIZE, MAX_PREVIEWSIZE);
        gtk_image_set_from_pixbuf(GTK_IMAGE(preview), pixbuf);
        g_object_unref(pixbuf);
        gtk_file_chooser_set_preview_widget_active(chooser, TRUE);
}


void model_select_all(GtkTreeModel *model)
{
        GtkTreeIter iter;

        if (!gtk_tree_model_get_iter_first(model, &iter))
                return;
        do {
                if (gtk_tree_model_get_flags(model) & GTK_TREE_MODEL_LIST_ONLY)
                        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, TRUE, -1);
                else
                        gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 0, TRUE, -1);
        } while (gtk_tree_model_iter_next(model, &iter));
}

void model_turn_all(GtkTreeModel *model)
{
        GtkTreeIter iter;
        gboolean active;

        if (!gtk_tree_model_get_iter_first(model, &iter))
                return;
        do {
                gtk_tree_model_get(model, &iter, 0, &active, -1);
                if (gtk_tree_model_get_flags(model) & GTK_TREE_MODEL_LIST_ONLY)
                        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, !active, -1);
                else
                        gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 0, !active, -1);
        } while (gtk_tree_model_iter_next(model, &iter));
}

void model_clear_all(GtkTreeModel *model)
{
        GtkTreeIter iter;

        if (!gtk_tree_model_get_iter_first(model, &iter))
                return;
        do {
                if (gtk_tree_model_get_flags(model) & GTK_TREE_MODEL_LIST_ONLY)
                        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, FALSE, -1);
                else
                        gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 0, FALSE, -1);
        } while (gtk_tree_model_iter_next(model, &iter));
}

void model_turn_select(GtkTreeModel *model, gchar *path)
{
        GtkTreeIter iter;
        gboolean active;

        if (!gtk_tree_model_get_iter_from_string(model, &iter, path))
                return;
        gtk_tree_model_get(model, &iter, 0, &active, -1);
        if (gtk_tree_model_get_flags(model) & GTK_TREE_MODEL_LIST_ONLY)
                gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, !active, -1);
        else
                gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 0, !active, -1);
}

void textview_follow_if_link(GtkWidget *textview, GtkTextIter *iter)
{
        GSList *tags, *tmp;
        GtkTextTag *tag;
        gchar *url;

        tmp = tags = gtk_text_iter_get_tags(iter);
        while (tmp) {
                tag = (GtkTextTag *)tmp->data;
                if ( (url = (gchar *)g_object_get_data(G_OBJECT(tag), "url"))) {
#if GTK_CHECK_VERSION(2,14,0)
                        if (!gtk_show_uri(NULL, url, GDK_CURRENT_TIME, NULL))
#endif
                                iptux_open_url(url);
                        break;
                }
                tmp = tmp->next;
        }
        g_slist_free(tags);
}

void textview_set_cursor_if_appropriate(GtkTextView *textview, gint x, gint y)
{
        GdkWindow *window;
        GSList *tags, *tmp;
        GtkTextTag *tag;
        GtkTextIter iter;
        gboolean hovering;

        hovering = FALSE;
        gtk_text_view_get_iter_at_location(textview, &iter, x, y);
        tmp = tags = gtk_text_iter_get_tags(&iter);
        while (tmp) {
                tag = (GtkTextTag *)tmp->data;
                if (g_object_get_data(G_OBJECT(tag), "url")) {
                        hovering = TRUE;
                        break;
                }
                tmp = tmp->next;
        }
        g_slist_free(tags);

        if (hovering != GPOINTER_TO_INT(
                 g_object_get_data(G_OBJECT(textview), "hovering-over-link"))) {
                window = gtk_text_view_get_window(textview, GTK_TEXT_WINDOW_TEXT);
                if (hovering)
                        gdk_window_set_cursor(window, progdt.lcursor);
                else
                        gdk_window_set_cursor(window, progdt.xcursor);
                g_object_set_data(G_OBJECT(textview), "hovering-over-link",
                                                 GINT_TO_POINTER(hovering));
        }
}

gboolean textview_key_press_event(GtkWidget *textview, GdkEventKey *event)
{
        GtkTextBuffer *buffer;
        GtkTextIter iter;
        gint position;

        switch (event->keyval) {
        case GDK_Return:
        case GDK_KP_Enter:
                buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
                g_object_get(buffer, "cursor-position", &position, NULL);
                gtk_text_buffer_get_iter_at_offset(buffer, &iter, position);
                textview_follow_if_link(textview, &iter);
                break;
        default:
                break;
        }

        return FALSE;
}

void textview_event_after(GtkWidget *textview, GdkEvent *ev)
{
        GtkTextBuffer *buffer;
        GdkEventButton *event;
        GtkTextIter iter;
        gboolean selected;
        gint x, y;

        if (ev->type != GDK_BUTTON_RELEASE)
                return;
        event = (GdkEventButton *)ev;
        if (event->button != 1)
                return;

        /* we shouldn't follow a link if the user has selected something */
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
        g_object_get(buffer, "has-selection", &selected, NULL);
        if (selected)
                return;

        gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(textview),
                 GTK_TEXT_WINDOW_WIDGET, event->x, event->y, &x, &y);
        gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(textview), &iter, x, y);
        textview_follow_if_link(textview, &iter);
}

gboolean textview_motion_notify_event(GtkWidget *textview, GdkEventMotion *event)
{
        gint x, y;

        gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(textview),
                 GTK_TEXT_WINDOW_WIDGET, event->x, event->y, &x, &y);
        textview_set_cursor_if_appropriate(GTK_TEXT_VIEW(textview), x, y);
        gdk_window_get_pointer(textview->window, NULL, NULL, NULL);

        return FALSE;
}

gboolean textview_visibility_notify_event(GtkWidget *textview, GdkEventVisibility *event)
{
        gint wx, wy, bx, by;

        gdk_window_get_pointer(textview->window, &wx, &wy, NULL);
        gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(textview),
                         GTK_TEXT_WINDOW_WIDGET, wx, wy, &bx, &by);
        textview_set_cursor_if_appropriate(GTK_TEXT_VIEW(textview), bx, by);

        return FALSE;
}
