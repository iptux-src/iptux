#include "IptuxConfig.h"

#include "deplib.h"

IptuxConfig::IptuxConfig(GConfClient* client) 
	: client(client)
{
	mainWindowWidth = gconf_client_get_int(client, GCONF_PATH "/main_window_width", NULL);
	mainWindowWidth = mainWindowWidth ? mainWindowWidth : 250;

	mainWindowHeight = gconf_client_get_int(client, GCONF_PATH "/main_window_height", NULL);
	mainWindowHeight = mainWindowHeight ? mainWindowHeight : 510;

	mwinMainPanedDivide = gconf_client_get_int(client, GCONF_PATH "/mwin_main_paned_divide", NULL);
	mwinMainPanedDivide = mwinMainPanedDivide ? mwinMainPanedDivide : 210;

	transWindowWidth = gconf_client_get_int(client, GCONF_PATH "/trans_window_width", NULL);
	transWindowWidth = transWindowWidth ? transWindowWidth : 500;

	transWindowHeight = gconf_client_get_int(client, GCONF_PATH "/trans_window_height", NULL);
	transWindowHeight = transWindowHeight ? transWindowHeight : 350;
}

IptuxConfig::~IptuxConfig() {
}

int IptuxConfig::GetTransWindowWidth() const {
	return transWindowWidth;
}

int IptuxConfig::GetTransWindowHeight() const {
	return transWindowHeight;
}

int IptuxConfig::GetMainWindowWidth() const {
	return mainWindowWidth;
}

int IptuxConfig::GetMainWindowHeight() const {
	return mainWindowHeight;
}

int IptuxConfig::GetMwinMainPanedDivide() const {
	return mwinMainPanedDivide;
}

IptuxConfig* IptuxConfig::SetMainWindowWidth(int w) {
	if(mainWindowWidth != w) {
		mainWindowWidth = w;
		gconf_client_set_int(client, GCONF_PATH "/main_window_width", mainWindowWidth, NULL);
	}
	return this;
}

IptuxConfig* IptuxConfig::SetMainWindowHeight(int h) {
	if(mainWindowHeight != h) {
		mainWindowHeight = h;
		gconf_client_set_int(client, GCONF_PATH "/main_window_height", mainWindowHeight, NULL);
	}
	return this;
}

IptuxConfig* IptuxConfig::SetTransWindowWidth(int w) {
	if(transWindowWidth != w) {
		transWindowWidth = w;
		gconf_client_set_int(client, GCONF_PATH "/trans_window_width", transWindowWidth, NULL);
	}
	return this;
}

IptuxConfig* IptuxConfig::SetTransWindowHeight(int h) {
	if(transWindowHeight != h) {
		transWindowHeight = h;
		gconf_client_set_int(client, GCONF_PATH "/transWindowHeight", transWindowHeight, NULL);
	}
	return this;
}

IptuxConfig* IptuxConfig::SetMwinMainPanedDivide(int d) {
	if(mwinMainPanedDivide != d) {
		mwinMainPanedDivide = d;
		gconf_client_set_int(client, GCONF_PATH "/mwin_main_paned_divide", mwinMainPanedDivide, NULL);
	}
	return this;
}

IptuxConfig* IptuxConfig::Save() {
	return this;
}