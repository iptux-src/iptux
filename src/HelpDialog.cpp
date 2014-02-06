//
// C++ Implementation: HelpDialog
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//          Jally <jallyx@163.com> & ManPT <pentie@gmail.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "HelpDialog.h"
#include "support.h"

/* 静态变量初始化 */
GtkWidget *HelpDialog::about = NULL;
GtkWidget *HelpDialog::more = NULL;
HelpDialog::HelpDialog()
{
}

HelpDialog::~HelpDialog()
{
}

/**
 * 关于对话框入口.
 */
void HelpDialog::AboutEntry()
{
        HelpDialog hlp;

        if (!about) {
                about = hlp.CreateAboutDialog();
                hlp.RunHelpDialog(&about);
        } else
                gtk_window_present(GTK_WINDOW(about));
}

/**
 * 更多信息对话框入口.
 */
void HelpDialog::MoreEntry()
{
        HelpDialog hlp;

        if (!more) {
                more = hlp.CreateMoreDialog();
                hlp.RunHelpDialog(&more);
        } else
                gtk_window_present(GTK_WINDOW(more));
}

/**
 * 创建关于对话框.
 */
GtkWidget *HelpDialog::CreateAboutDialog()
{
        const char *authors[] = {
                _("Jally <jallyx@163.com>"),
                _("ManPT <pentie@gmail.com>"),
                NULL
        };
        const char *artists[] = {
                _("Jally <jallyx@163.com>"),
                _("LiWeijian <weijian_li88@qq.com>"),
                _("ManPT <pentie@gmail.com>"),
                NULL
        };
        const char *translators = _("alick <alick9188@163.com>\n"
                                     "ManPT <pentie@gmail.com>");
        GtkWidget *dialog;

        dialog = gtk_about_dialog_new();
        gtk_about_dialog_set_email_hook(
                 GtkAboutDialogActivateLinkFunc(DialogOpenEmail), NULL, NULL);
        gtk_about_dialog_set_url_hook(
                 GtkAboutDialogActivateLinkFunc(DialogOpenUrl), NULL, NULL);
        gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), _("iptux"));
        gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), VERSION);
        gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog),
                                         "Copyright © 2008-2009 by Jally");
        gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog),
                                         _("A GTK+ based LAN Messenger."));
        gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog),
                                         "https://github.com/iptux-src/iptux");
        gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(dialog), "GPL 2+");
        gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);
        gtk_about_dialog_set_artists(GTK_ABOUT_DIALOG(dialog), artists);
        gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(dialog), translators);
        gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(dialog), "ip-tux");

        return dialog;
}

/**
 * 创建更多信息对话框.
 */
GtkWidget *HelpDialog::CreateMoreDialog()
{
        char *labels[] = {_("Help"), _("Contributers"), _("..."), NULL};
        char *text[] = {
                _("Project Home: \nhttps://github.com/iptux-src/iptux\n\n"
                  "User and Developer Group: \nhttps://groups.google.com/group/iptux/\n\n"
                  "Note that you can get help form the project wiki page.\n\n"
                  "If you find no solutions in any of the existed documents, "
                  "feel free to drop a email at iptux@googlegroups.com, "
                  "lots of users and developers would be glade to help about your problems."),
                _("It's an honor that iptux was contributed voluntarilly by many people. "
                  "Without your help, iptux could never make it.\n\n"
                  "Especially, Here's some we would like to thank much:\n\n"
                  "ChenGang\n"
                  "<liangsuilong@gmail.com>\n"
                  "<lidaobing@gmail.com>\n"
                  "<mdjhu@sina.com>\n"
                  "<omegao.hu@gmail.com>\n"
                  "<syranosun@gmail.com>\n"
                  "..."),
                _("...")
        };
        GtkWidget *dialog;
        GtkWidget *frame, *sw;
        GtkWidget *notebook, *label, *view;
        GtkTextBuffer *buffer;
        gint page;

        dialog = gtk_dialog_new_with_buttons(_("More About Iptux"), NULL,
                                 GTK_DIALOG_NO_SEPARATOR,
                                 GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
        gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
        gtk_widget_set_size_request(dialog, 500, 350);

        frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame, TRUE, TRUE, 0);
        gtk_container_set_border_width(GTK_CONTAINER(frame), 3);

        notebook = gtk_notebook_new();
        gtk_container_add(GTK_CONTAINER(frame), notebook);

        page = 0;
        while (labels[page]) {
                sw = gtk_scrolled_window_new(NULL, NULL);
                gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
                gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                                         GTK_SHADOW_ETCHED_IN);
                label = gtk_label_new(labels[page]);
                gtk_notebook_append_page(GTK_NOTEBOOK(notebook), sw, label);
                view = gtk_text_view_new();
                gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD);
                gtk_container_add(GTK_CONTAINER(sw), view);

                gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
                gtk_text_view_set_indent(GTK_TEXT_VIEW(view), 20);
                gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(view), 5);
                gtk_text_view_set_editable (GTK_TEXT_VIEW(view), FALSE);

                buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
                gtk_text_buffer_set_text(buffer, text[page], -1);

                page++;
        }

        return dialog;
}

/**
 * 运行对话框.
 * @param dialog 对话框指针
 */
void HelpDialog::RunHelpDialog(GtkWidget **dialog)
{
        g_signal_connect(*dialog, "close", G_CALLBACK(gtk_widget_destroy), NULL);
        g_signal_connect(*dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
        g_signal_connect_swapped(*dialog, "destroy", G_CALLBACK(DialogDestroy), dialog);
        gtk_window_set_skip_taskbar_hint(GTK_WINDOW(*dialog), TRUE);
        gtk_window_set_skip_pager_hint(GTK_WINDOW(*dialog), TRUE);
        gtk_widget_show_all(*dialog);
}

/**
 * 打开email.
 * @param dialog 对话框
 * @param link email
 */
void HelpDialog::DialogOpenEmail(GtkWidget *dialog, const gchar *link)
{
        const char *prefix = "mailto:";
        gchar *ptr;

        if (strncasecmp(link, prefix, strlen(prefix)) != 0)
                ptr = g_strdup_printf("%s%s", prefix, link);
        else
                ptr = g_strdup(link);
#if GTK_CHECK_VERSION(2,14,0)
        if (!gtk_show_uri(NULL, ptr, GDK_CURRENT_TIME, NULL))
#endif
                iptux_open_url(ptr);
        g_free(ptr);
}

/**
 * 打开URL.
 * @param dialog 对话框
 * @param link URL
 */
void HelpDialog::DialogOpenUrl(GtkWidget *dialog, const gchar *link)
{
#if GTK_CHECK_VERSION(2,14,0)
        if (!gtk_show_uri(NULL, link, GDK_CURRENT_TIME, NULL))
#endif
                iptux_open_url(link);
}

/**
 * 对话框被摧毁的响应函数.
 * @param dialog 对话框指针
 */
void HelpDialog::DialogDestroy(GtkWidget **dialog)
{
        *dialog = NULL;
}
