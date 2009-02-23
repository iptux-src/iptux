//
// C++ Interface: Sound
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef sound_h
#define sound_h

#include "sys.h"
#include "face.h"

class Sound
{
 public:
	Sound();
	~Sound();

	void InitSelf();
	void Playing(const char *file);
	void Stop();
#ifdef HAVE_GST
 private:
	GstElement *pipeline,*filesrc, *decode, *convert;
	struct timeval timestamp;	//时间间隔过短则忽略后一个要求
	bool persist;
 private:
	 static void NewDecodedPad(GstElement *convert, GstPad* pad);
	static void ErrorMessageOccur(pointer data, GstMessage *message);	//Sound
	static void EosMessageOccur(pointer data);	//
#endif
};

#endif
