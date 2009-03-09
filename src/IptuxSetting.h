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
	void CreateSound(GtkWidget * note);
	void CreateIpseg(GtkWidget * note);
	void CreateFuncButton(GtkWidget * hbb);
	GtkTreeModel *CreateSndModel();
	GtkTreeModel *CreateIpModel();
	GtkWidget *CreateFontChooser();
	GtkWidget *CreateSndChooser();
	GtkWidget *CreateSndView();
	GtkWidget *CreateIpsegView();
	static bool CheckExist();

	GtkTreeModel *icon_model, *snd_model, *ip_model;
	GtkWidget *myname, *mygroup, *myicon, *save_path, *ad, *sign;
	GtkWidget *encode, *palicon, *font, *memory, *etrkey, *tidy,
						 *log, *black, *proof;
	GtkWidget *sound, *volume;
	GtkWidget *entry1, *entry2, *ipseg_view;
	static GtkWidget *setting;
 public:
	static GtkWidget *CreateArchiveChooser();
	static GtkTreeModel *CreateIconModel();
	static GtkWidget *CreateComboBoxWithModel(GtkTreeModel * model,
						  gchar * iconfile);
	static gint FileGetItemPos(const char *filename, GtkTreeModel * model);
 private:
	void ObtainPerson();
	void ObtainSystem();
	void ObtainSound();
	void ObtainIpseg();
	void UpdateMyInfo();
	void UpdateNetSegment(const char *filename, GSList ** list, bool dirc);
//回调处理部分
 public:
	static void AddPalIcon(GtkWidget * combo);
 private:
	 static void ChoosePortrait(GtkWidget * image);

	static void AdjustSensitive(GtkWidget *sound, GtkWidget *widget);
	static void AdjustVolume(GtkWidget *volume);
	static void SelectItemChanged(GtkTreeSelection *selection,
				      GtkWidget *chooser);
	static void ChooserResetModel(GtkWidget *chooser,
				      GtkTreeSelection *selection);
	static void PlayTesting(GtkWidget *chooser);
	static void StopTesting();

	static void ClickAddIpseg(gpointer data);	//IptuxSetting
	static void ClickDelIpseg(gpointer data);	//
	static void CellEditText(GtkCellRendererText * renderer, gchar * path,
				 gchar * new_text, GtkTreeModel * model);
	static void ImportNetSegment(gpointer data);	//
	static void ExportNetSegment(gpointer data);	//

	static void ClickOk(gpointer data);	//
	static void ClickApply(gpointer data);	//
	static void SettingDestroy(gpointer data);	//
};

#endif
