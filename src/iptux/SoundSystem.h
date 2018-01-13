//
// C++ Interface: SoundSystem
//
// Description:
// 声音系统
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_SOUNDSYSTEM_H
#define IPTUX_SOUNDSYSTEM_H

#include "iptux/config.h"
#ifdef GST_FOUND
# include <gst/gst.h>
#endif

#include "iptux/mess.h"

namespace iptux {

class SoundSystem
{
public:
        SoundSystem();
        ~SoundSystem();

        void InitSublayer();
        void AdjustVolume(double value);
        void Playing(const char *file);
        void Stop();
#ifdef GST_FOUND
private:
        GData *eltset;          //元素集
        struct timeval timestamp;       //时间戳
        bool persist;   //声音系统占用标记
private:
        static void LinkElement(GData **eltset, GstPad *pad);
        static void ErrorMessageOccur(SoundSystem *sndsys, GstMessage *message);
        static void EosMessageOccur(SoundSystem *sndsys);
#endif
};

}

#endif
