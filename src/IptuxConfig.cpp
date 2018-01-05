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

	ifstream ifs(fname.c_str());
	if(!ifs.is_open()) {
		g_warning("config file %s not found", fname.c_str());
		return;
	}


	Json::CharReaderBuilder rbuilder;
	std::string errs;
	bool ok = Json::parseFromStream(rbuilder, ifs, &root, &errs);
	if(!ok) {
		g_warning("invalid content in config file %s:\n%s", fname.c_str(),errs.c_str());
		return;
	}

	int version = root.get("version", 1).asInt();
	if(version != 1) {
		g_error("unknown config file version %d (from %s)", version, fname.c_str());
		return;
	}

	transWindowWidth = root.get("trans_window_width", 500).asInt();
	transWindowHeight = root.get("trans_window_height", 350).asInt();
	mwinMainPanedDivide = root.get("mwin_main_paned_divide", 210).asInt();
	accessSharedLimit = root.get("access_shared_limit", "").asString();

	const Json::Value l = root["shared_file_list"];
	for(int i = 0; i < int(l.size()); ++i) {
		sharedFileList.push_back(l[i].asString());
	}
}

IptuxConfig::~IptuxConfig() {
}

int IptuxConfig::GetInt(const string& key) const {
	return root.get(key, 0).asInt();
}

void IptuxConfig::SetInt(const string& key, int value) {
	root[key] = value;
}

string IptuxConfig::GetString(const string& key) const {
  return GetString(key, "");
}

string IptuxConfig::GetString(const string& key, const string& defaultValue) const {
  return root.get(key, defaultValue).asString();
}

void IptuxConfig::SetString(const string& key, const string& value) {
	root[key] = value;
}

int IptuxConfig::GetTransWindowWidth() const {
	return transWindowWidth;
}

int IptuxConfig::GetTransWindowHeight() const {
	return transWindowHeight;
}

int IptuxConfig::GetMwinMainPanedDivide() const {
	return mwinMainPanedDivide;
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

	root["version"] = 1;
	root["trans_window_width"] = transWindowWidth;
	root["trans_window_height"] = transWindowHeight;
	root["mwin_main_paned_divide"] = mwinMainPanedDivide;
	root["access_shared_limit"] = accessSharedLimit;
	for(size_t i = 0; i < sharedFileList.size(); ++i) {
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
