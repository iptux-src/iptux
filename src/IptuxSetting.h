//
// C++ Interface: IptuxSetting
//
// Description:程序功能、数据设置
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUXSETTING_H
#define IPTUXSETTING_H

#include "udt.h"

class IptuxSetting {
 public:
	IptuxSetting();
	~IptuxSetting();

	static void SettingEntry();
 private:
	void InitSetting();
	void CreateSetting();
	void CreatePerson(GtkWidget * note);
	void CreateSystem(GtkWidget * note);
	void CreateIpseg(GtkWidget * note);
	void CreateFuncButton(GtkWidget * hbb);
	GtkTreeModel *CreateIpModel();
	GtkWidget *CreateFolderChooser();
	GtkWidget *CreateFontChooser();
	GtkWidget *CreateIpsegView();
	static bool CheckExist();

	GtkTreeModel *icon_model, *ip_model;
	GtkWidget *myname, *mygroup, *myicon, *save_path, *ad, *sign;
	GtkWidget *encode, *palicon, *font, *sound, *memory, *etrkey,
					 *tidy, *log, *black, *proof;
	GtkWidget *entry1, *entry2, *ipseg_view;
	static GtkWidget *setting;
 public:
	static GtkTreeModel *CreateIconModel();
	static GtkWidget *CreateComboBoxWithModel(GtkTreeModel * model,
						  gchar * iconfile);
	static gint FileGetItemPos(const char *filename, GtkTreeModel * model);
 private:
	void ObtainPerson();
	void ObtainSystem();
	void ObtainIpseg();
	void UpdateMyInfo();
	void UpdateNetSegment(const char *filename, GSList ** list, bool dirc);
//回调处理部分
 public:
	static void AddPalIcon(GtkWidget * combo);
 private:
	static void ClickAddIpseg(gpointer data);	//IptuxSetting
	static void ClickDelIpseg(gpointer data);	//
	static void ClickOk(gpointer data);	//
	static void ClickApply(gpointer data);	//
	static void SettingDestroy(gpointer data);	//

	static void ChoosePortrait(GtkWidget * image);
	static void CellEditText(GtkCellRendererText * renderer, gchar * path,
				 gchar * new_text, GtkTreeModel * model);
	static void ImportNetSegment(gpointer data);	//
	static void ExportNetSegment(gpointer data);	//
};

#endif
