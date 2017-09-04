#include "IptuxConfig.h"

#include <fstream>

#include <glib.h>
#include <glib/gstdio.h>
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
		g_error("unknown config file version %d (from %s)", version, fname.c_str());
		return;
	}

	mainWindowWidth = root.get("main_window_width", 250).asInt();
	mainWindowHeight = root.get("main_window_height", 510).asInt();
	transWindowWidth = root.get("trans_window_width", 500).asInt();
	transWindowHeight = root.get("trans_window_height", 350).asInt();
	mwinMainPanedDivide = root.get("mwin_main_paned_divide", 210).asInt();
	accessSharedLimit = root.get("access_shared_limit", "").asString();

	const Json::Value l = root["shared_file_list"];
	for(int i = 0; i < l.size(); ++i) {
		sharedFileList.push_back(l[i].asString());
	}
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
	if(!g_file_test(fname.c_str(), G_FILE_TEST_IS_REGULAR)) {
		const char* dirname = g_path_get_dirname(fname.c_str());
		if(g_mkdir_with_parents(dirname, 0700) != 0) {
			g_error("create config dir %s failed: %s", dirname, strerror(errno));
		}
	}

	Json::Value root;
	root["version"] = 1;
	root["main_window_width"] = mainWindowWidth;
	root["main_window_height"] = mainWindowHeight;
	root["trans_window_width"] = transWindowWidth;
	root["trans_window_height"] = transWindowHeight;
	root["mwin_main_paned_divide"] = mwinMainPanedDivide;
	root["access_shared_limit"] = accessSharedLimit;
	for(int i = 0; i < sharedFileList.size(); ++i) {
		root["shared_file_list"].append(sharedFileList[i]);
	}

	ofstream ofs(fname.c_str());
	if(!ofs) {
		g_warning("open config file %s for write failed.", fname.c_str());
		return this;
	}
	ofs << root;
	if(!ofs) {
		g_warning("write to config file %s failed.", fname.c_str());
	}
	return this;
}