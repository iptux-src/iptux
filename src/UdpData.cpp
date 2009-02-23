//
// C++ Implementation: UdpData
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "UdpData.h"
#include "MainWindow.h"
#include "SendFile.h"
#include "Control.h"
#include "Sound.h"
#include "Command.h"
#include "Pal.h"
#include "dialog.h"
#include "baling.h"
#include "utils.h"

 UdpData::UdpData():pallist(NULL), msgqueue(NULL)
{
	pthread_mutex_init(&mutex, NULL);
}

UdpData::~UdpData()
{
	pthread_mutex_lock(&mutex);
	g_slist_foreach(pallist, GFunc(remove_foreach),
			GINT_TO_POINTER(PALINFO));
	g_slist_free(pallist);
	g_queue_clear(msgqueue);
	g_queue_free(msgqueue);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
}

void UdpData::InitSelf()
{
	msgqueue = g_queue_new();
}

void UdpData::AdjustMemory()
{
	extern Control ctr;
	GSList *tmp, *list;
	Pal *pal;

	//防止申请锁的方向出现错误
	pthread_mutex_lock(&mutex);
	tmp = list = g_slist_copy(pallist);	//根据其只增不减的特性，可以采用浅拷贝方案
	pthread_mutex_unlock(&mutex);

	while (tmp) {
		pal = (Pal *) tmp->data;
		gdk_threads_enter();
		if (FLAG_ISSET(ctr.flags, 5)) {	//最小化内存使用
			if (pal->IconpixQuote()) {
				g_object_unref(pal->IconpixQuote());
				pal->IconpixQuote() = NULL;
			}
		} else {	//以内存换取运行效率
			if (!pal->IconpixQuote()) {
				pal->IconpixQuote() = pal->GetIconPixbuf();	//返回对象必须被释放
				g_object_unref(pal->IconpixQuote());
			}
		}
		gdk_threads_leave();
		tmp = tmp->next;
	}
	g_slist_free(list);
}

void UdpData::MsgBlinking()
{
	extern MainWindow *mwp;
	GtkTreeIter iter;
	GList *tmp;
	guint count, length;

	gdk_threads_enter();
	pthread_mutex_lock(&mutex);
	count = 0;
	length = msgqueue->length;
	tmp = msgqueue->head;
	while (count < length) {
		if (mwp->PalGetModelIter(tmp->data, &iter))
			mwp->MakeItemBlinking(&iter, true);
		tmp = tmp->next;
		count++;
	}
	pthread_mutex_unlock(&mutex);
	gdk_threads_leave();
}

void UdpData::UdpDataEntry(in_addr_t ipv4, char *msg, size_t size)
{
	extern Control ctr;
	uint32_t commandno;
	Pal *pal;

	if (FLAG_ISSET(ctr.flags, 1) && (pal = (Pal *) Ipv4GetPal(ipv4))
		   && FLAG_ISSET(pal->FlagsQuote(), 3))
		return;
	commandno = iptux_get_dec_number(msg, 4);
	switch (GET_MODE(commandno)) {
	case IPMSG_BR_ENTRY:
		SomeoneEntry(ipv4, msg, size);
		break;
	case IPMSG_BR_EXIT:
		SomeoneExit(ipv4, msg, size);
		break;
	case IPMSG_ANSENTRY:
		SomeoneAnsentry(ipv4, msg, size);
		break;
	case IPMSG_BR_ABSENCE:
		SomeoneAbsence(ipv4, msg, size);
		break;
	case IPMSG_SENDMSG:
		SomeoneSendmsg(ipv4, msg, size);
		break;
	case IPMSG_RECVMSG:
		SomeoneRecvmsg(ipv4, msg, size);
		break;
	case IPTUX_ASKSHARED:
		SomeoneAskShared(ipv4, msg, size);
		break;
	case IPTUX_SENDICON:
		SomeoneSendIcon(ipv4, msg, size);
		break;
	case IPTUX_SENDSIGN:
		SomeoneSendSign(ipv4, msg, size);
		break;
	default:
		break;
	}
}

void UdpData::SublayerEntry(gpointer data, uint32_t command, const char *path)
{
	Pal *pal;

	pal = (Pal *) data;
	switch (GET_OPT(command)) {
	case IPTUX_ADPICOPT:
		pal->RecvAdPic(path);
		break;
	case IPTUX_MSGPICOPT:
		pal->RecvMsgPic(path);
		break;
	}
}

gpointer UdpData::Ipv4GetPal(in_addr_t ipv4)
{
	extern UdpData udt;
	GSList *tmp;
	Pal *pal;

	pthread_mutex_lock(&udt.mutex);
	tmp = udt.pallist;
	while (tmp) {
		pal = (Pal *) tmp->data;
		if (ipv4 == pal->Ipv4Quote())
			break;
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&udt.mutex);

	if (tmp)
		return pal;
	return NULL;
}

gpointer UdpData::Ipv4GetPalPos(in_addr_t ipv4)
{
	extern UdpData udt;
	GSList *tmp;
	Pal *pal;

	pthread_mutex_lock(&udt.mutex);
	tmp = udt.pallist;
	while (tmp) {
		pal = (Pal *) tmp->data;
		if (ipv4 == pal->Ipv4Quote())
			break;
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&udt.mutex);

	return tmp;
}

gpointer UdpData::PalGetMsgPos(pointer data)
{
	extern UdpData udt;
	GList *tmp;
	guint count, length;

	pthread_mutex_lock(&udt.mutex);
	count = 0;
	length = udt.msgqueue->length;
	tmp = udt.msgqueue->head;
	while (count < length) {
		if (tmp->data == data)
			break;
		tmp = tmp->next;
		count++;
	}
	pthread_mutex_unlock(&udt.mutex);

	if (count < length)
		return tmp;
	return NULL;
}

void UdpData::SomeoneLost(in_addr_t ipv4, char *msg, size_t size)
{
	extern Control ctr;
	Pal *pal;

	pal = new Pal;
	pthread_mutex_lock(&mutex);
	pallist = g_slist_append(pallist, pal);
	pthread_mutex_unlock(&mutex);

	//对底层调用函数中可能不会被初始化的数据进行预初始化
	pal->Ipv4Quote() = ipv4;
	pal->NameQuote() = Strdup(_("mysterious"));
	pal->GroupQuote() = Strdup(_("mysterious"));
	pal->IconfileQuote() = Strdup(ctr.palicon);
	pal->EncodeQuote() = Strdup(ctr.encode);
	FLAG_SET(pal->FlagsQuote(), 2);

	SomeoneAbsence(ipv4, msg, size);
}

void UdpData::SomeoneEntry(in_addr_t ipv4, char *msg, size_t size)
{
	extern MainWindow *mwp;
	GtkTreeIter iter;
	Pal *pal;

	pal = (Pal *) Ipv4GetPal(ipv4);
	gdk_threads_enter();
	if (!pal) {
		pal = new Pal;
		pthread_mutex_lock(&mutex);
		pallist = g_slist_append(pallist, pal);
		pthread_mutex_unlock(&mutex);
		pal->CreateInfo(ipv4, msg, size, true);
		mwp->AttachItemToModel(ipv4, &iter);
	} else {
		pal->UpdateInfo(msg, size, true);
		if (!mwp->PalGetModelIter(pal, &iter))
			mwp->AttachItemToModel(ipv4, &iter);
	}
	mwp->SetValueToModel(pal, &iter);
	gdk_threads_leave();
	pal->SendAnsentry();
	if (FLAG_ISSET(pal->FlagsQuote(), 0))
		thread_create(ThreadFunc(Pal::SendFeature), pal, false);
}

void UdpData::SomeoneExit(in_addr_t ipv4, char *msg, size_t size)
{
	extern MainWindow *mwp;
	GSList *tmp1;
	GList *tmp2;

	tmp1 = (GSList *) Ipv4GetPalPos(ipv4);
	if (!tmp1)
		return;

	gdk_threads_enter();
	mwp->DelItemFromModel(tmp1->data);
	gdk_threads_leave();
	tmp2 = (GList *) PalGetMsgPos(tmp1->data);
	if (tmp2) {
		pthread_mutex_lock(&mutex);
		g_queue_delete_link(msgqueue, tmp2);
		pthread_mutex_unlock(&mutex);
	}
	FLAG_CLR(((Pal *) tmp1->data)->FlagsQuote(), 1);
}

void UdpData::SomeoneAnsentry(in_addr_t ipv4, char *msg, size_t size)
{
	Pal *pal;

	SomeoneAbsence(ipv4, msg, size);
	if (!(pal = (Pal *) Ipv4GetPal(ipv4)))
		return;
	if (FLAG_ISSET(pal->FlagsQuote(), 0))
		thread_create(ThreadFunc(Pal::SendFeature), pal, false);
}

void UdpData::SomeoneAbsence(in_addr_t ipv4, char *msg, size_t size)
{
	extern MainWindow *mwp;
	GtkTreeIter iter;
	Pal *pal;

	pal = (Pal *) Ipv4GetPal(ipv4);
	gdk_threads_enter();
	if (!pal) {
		pal = new Pal;
		pthread_mutex_lock(&mutex);
		pallist = g_slist_append(pallist, pal);
		pthread_mutex_unlock(&mutex);
		pal->CreateInfo(ipv4, msg, size, false);
		mwp->AttachItemToModel(ipv4, &iter);
	} else {
		pal->UpdateInfo(msg, size, false);
		if (!mwp->PalGetModelIter(pal, &iter))
			mwp->AttachItemToModel(ipv4, &iter);
	}
	mwp->SetValueToModel(pal, &iter);
	gdk_threads_leave();
}

void UdpData::SomeoneSendmsg(in_addr_t ipv4, char *msg, size_t size)
{
	extern struct interactive inter;
	extern Control ctr;
	extern Sound sound;
	Command cmd;
	char *passwd, *text;
	uint32_t commandno;
	bool flag;
	Pal *pal;

	pal = (Pal *) Ipv4GetPal(ipv4);
	if (!pal) {
		SomeoneLost(ipv4, msg, size);
		if (!(pal = (Pal *) Ipv4GetPal(ipv4)))
			return;
	}

	flag = pal->RecvMessage(msg);
	commandno = iptux_get_dec_number(msg, 4);
	if (commandno & IPMSG_SENDCHECKOPT)
		pal->SendReply(msg);
	if (flag && FLAG_ISSET(ctr.sndfgs, 1)
		   && !(commandno & IPMSG_FILEATTACHOPT))
		sound.Playing(ctr.msgtip);
	if (flag && (commandno & IPMSG_FILEATTACHOPT)) {
		if ((commandno & IPTUX_SHAREDOPT)
			     && (commandno & IPTUX_PASSWDOPT)) {
			passwd = pop_obtain_passwd();
			if (passwd && *passwd != '\0') {
				text = g_base64_encode((guchar *)passwd,
							strlen(passwd));
				cmd.SendAskShared(inter.udpsock, pal,
						IPTUX_PASSWDOPT, text);
				free(text);
			}
			free(passwd);
		} else
			pal->RecvFile(msg, size);
	}
}

void UdpData::SomeoneRecvmsg(in_addr_t ipv4, char *msg, size_t size)
{
	Pal *pal;

	pal = (Pal *) Ipv4GetPal(ipv4);
	if (!pal)
		return;

	pal->RecvReply(msg);
}

void UdpData::SomeoneAskShared(in_addr_t ipv4, char *msg, size_t size)
{
	extern struct interactive inter;
	extern SendFile sfl;
	Command cmd;
	char *passwd;
	Pal *pal;

	pal = (Pal *) Ipv4GetPal(ipv4);
	if (!pal || !pal->RecvAskShared(msg))
		return;

	if (*sfl.PasswdQuote() == '\0')
		thread_create(ThreadFunc(ThreadAskShared), pal, false);
	else if (!(iptux_get_dec_number(msg, 4) & IPTUX_PASSWDOPT))
		cmd.SendFileInfo(inter.udpsock, pal,
				 IPTUX_SHAREDOPT | IPTUX_PASSWDOPT, "");
	else {
		if ((passwd = ipmsg_get_attach(msg, 5))
		    && (strcmp(passwd, sfl.PasswdQuote()) == 0))
			thread_create(ThreadFunc(ThreadAskShared), pal, false);
		free(passwd);
	}
}

void UdpData::SomeoneSendIcon(in_addr_t ipv4, char *msg, size_t size)
{
	extern MainWindow *mwp;
	GtkTreeIter iter;
	Pal *pal;
	bool flag;

	pal = (Pal *) Ipv4GetPal(ipv4);
	if (!pal)
		return;

	flag = pal->RecvIcon(msg, size);
	if (flag) {
		gdk_threads_enter();
		mwp->PalGetModelIter(pal, &iter);
		mwp->SetValueToModel(pal, &iter);
		gdk_threads_leave();
	}
}

void UdpData::SomeoneSendSign(in_addr_t ipv4, char *msg, size_t size)
{
	Pal *pal;

	pal = (Pal *) Ipv4GetPal(ipv4);
	if (!pal) {
		SomeoneLost(ipv4, msg, size);
		if (!(pal = (Pal *) Ipv4GetPal(ipv4)))
			return;
	}
	pal->RecvSign(msg);
}

void UdpData::ThreadAskShared(gpointer data)
{
	extern Control ctr;
	extern SendFile sfl;

	if (!FLAG_ISSET(ctr.flags, 0) || pop_request_shared(data))
		sfl.SendSharedInfo(data);
}
