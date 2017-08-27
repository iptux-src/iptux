#include "IptuxConfig.h"

#include <fstream>
#include <json/json.h>
#include "deplib.h"

using namespace std;

IptuxConfig::IptuxConfig(string& fname) 
	: fname(fname)
{

	Json::Value root;

	ifstream ifs(fname.c_str());
	if(ifs.is_open()) {
		ifs >> root;
	} else {
		g_warning("config file %s not found", fname.c_str());
	}

	int version = root.get("version", 1).asInt();
	if(version != 1) {
		g_error("unknown config file version %d", version);
		return;
	}

	mainWindowWidth = root.get("main_window_width", 250).asInt();
	mainWindowHeight = root.get("main_window_height", 510).asInt();
	transWindowWidth = root.get("trans_window_width", 500).asInt();
	transWindowHeight = root.get("trans_window_height", 350).asInt();
	mwinMainPanedDivide = root.get("mwin_main_paned_divide", 210).asInt();
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
	}
	return this;
}

IptuxConfig* IptuxConfig::SetMainWindowHeight(int h) {
	if(mainWindowHeight != h) {
		mainWindowHeight = h;
	}
	return this;
}

IptuxConfig* IptuxConfig::SetTransWindowWidth(int w) {
	if(transWindowWidth != w) {
		transWindowWidth = w;
	}
	return this;
}

IptuxConfig* IptuxConfig::SetTransWindowHeight(int h) {
	if(transWindowHeight != h) {
		transWindowHeight = h;
	}
	return this;
}

IptuxConfig* IptuxConfig::SetMwinMainPanedDivide(int d) {
	if(mwinMainPanedDivide != d) {
		mwinMainPanedDivide = d;
	}
	return this;
}

IptuxConfig* IptuxConfig::Save() {
	Json::Value root;
	root["version"] = 1;
	root["main_window_width"] = mainWindowWidth;
	root["main_window_height"] = mainWindowHeight;
	root["trans_window_width"] = transWindowWidth;
	root["trans_window_height"] = transWindowHeight;
	root["mwin_main_paned_divide"] = mwinMainPanedDivide;
	ofstream ofs(fname);
	ofs << root;
	return this;
}