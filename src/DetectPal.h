//
// C++ Interface: DetectPal
//
// Description:探测好友
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DETECTPAL_H
#define DETECTPAL_H

#include "deplib.h"

class DetectPal {
public:
        DetectPal();
        ~DetectPal();

        static void DetectEntry(GtkWidget *parent);
private:
        GtkWidget *CreateMainDialog(GtkWidget *parent);
        GtkWidget *CreateInputArea();
        void SendDetectPacket();

        GData *widset;
};

#endif
