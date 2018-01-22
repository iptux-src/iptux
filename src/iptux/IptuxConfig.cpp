#include "IptuxConfig.h"

#include <cstring>
#include <fstream>

#include <glib.h>
#include <glib/gstdio.h>
#include <json/json.h>

using namespace std;

namespace iptux {

IptuxConfig::IptuxConfig(string& fname) : fname(fname) {
  ifstream ifs(fname.c_str());
  if (!ifs.is_open()) {
    g_warning("config file %s not found", fname.c_str());
    return;
  }

  Json::CharReaderBuilder rbuilder;
  std::string errs;
  bool ok = Json::parseFromStream(rbuilder, ifs, &root, &errs);
  if (!ok) {
    g_warning("invalid content in config file %s:\n%s", fname.c_str(),
              errs.c_str());
    return;
  }

  int version = root.get("version", 1).asInt();
  if (version != 1) {
    g_error("unknown config file version %d (from %s)", version, fname.c_str());
    return;
  }
}

IptuxConfig::~IptuxConfig() {}

int IptuxConfig::GetInt(const string& key) const { return GetInt(key, 0); }

int IptuxConfig::GetInt(const string& key, int defaultValue) const {
  return root.get(key, defaultValue).asInt();
}

void IptuxConfig::SetInt(const string& key, int value) { root[key] = value; }

bool IptuxConfig::GetBool(const string& key) const {
  return root.get(key, false).asBool();
}
void IptuxConfig::SetBool(const string& key, bool value) { root[key] = value; }

string IptuxConfig::GetString(const string& key) const {
  return GetString(key, "");
}

string IptuxConfig::GetString(const string& key,
                              const string& defaultValue) const {
  return root.get(key, defaultValue).asString();
}

void IptuxConfig::SetString(const string& key, const string& value) {
  root[key] = value;
}

double IptuxConfig::GetDouble(const string& key) const {
  return root.get(key, 0.0).asDouble();
}

void IptuxConfig::SetDouble(const string& key, double value) {
  root[key] = value;
}

vector<string> IptuxConfig::GetStringList(const string& key) const {
  vector<string> res;
  Json::Value value = root[key];
  if (value.isNull()) {
    return res;
  }
  if (value.isArray()) {
    for (size_t i = 0; i < value.size(); ++i) {
      res.push_back(value.get(i, "").asString());
    }
  }
  return res;
}

void IptuxConfig::SetStringList(const string& key,
                                const vector<string>& value) {
  root[key] = Json::arrayValue;
  for (size_t i = 0; i < value.size(); ++i) {
    root[key][int(i)] = value[i];
  }
}

void IptuxConfig::SetVector(const string& key,
                            const vector<Json::Value>& value) {
  for (size_t i = 0; i < value.size(); ++i) {
    root[key][int(i)] = value[i];
  }
}

vector<Json::Value> IptuxConfig::GetVector(const string& key) const {
  vector<Json::Value> res;
  Json::Value value = root[key];
  if (value.isNull()) {
    return res;
  }
  if (value.isArray()) {
    for (size_t i = 0; i < value.size(); ++i) {
      res.push_back(value[int(i)]);
    }
  }
  return res;
}

IptuxConfig& IptuxConfig::Save() {
  if (!g_file_test(fname.c_str(), G_FILE_TEST_IS_REGULAR)) {
    const char* dirname = g_path_get_dirname(fname.c_str());
    if (g_mkdir_with_parents(dirname, 0700) != 0) {
      g_error("create config dir %s failed: %s", dirname, strerror(errno));
    }
  }

  root["version"] = 1;

  ofstream ofs(fname.c_str());
  if (!ofs) {
    g_warning("open config file %s for write failed.", fname.c_str());
    return *this;
  }
  ofs << root;
  if (!ofs) {
    g_warning("write to config file %s failed.", fname.c_str());
  }
  return *this;
}

}  // namespace iptux
