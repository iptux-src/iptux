#ifndef IPTUX_IPTUX_CONFIG_H
#define IPTUX_IPTUX_CONFIG_H

#include <string>
#include <vector>

#include <json/json.h>

namespace iptux {

class IptuxConfig {
 public:
  explicit IptuxConfig(const std::string& fname);
  ~IptuxConfig();

  const std::string& getFileName() const;

  int GetInt(const std::string& key) const;
  int GetInt(const std::string& key, int defaultValue) const;
  void SetInt(const std::string& key, int value);

  std::string GetString(const std::string& key) const;
  std::string GetString(const std::string& key,
                        const std::string& defaultValue) const;
  void SetString(const std::string& key, const std::string& value);

  bool GetBool(const std::string& key) const;
  bool GetBool(const std::string& key, bool defaultValue) const;
  void SetBool(const std::string& key, bool value);

  double GetDouble(const std::string& key) const;
  double GetDouble(const std::string& key, double defaultValue) const;
  void SetDouble(const std::string& key, double value);

  std::vector<std::string> GetStringList(const std::string& key) const;
  void SetStringList(const std::string& key,
                     const std::vector<std::string>& value);

  std::vector<Json::Value> GetVector(const std::string& key) const;
  void SetVector(const std::string& key, const std::vector<Json::Value>& value);

  IptuxConfig& Save();

 private:
  std::string fname;
  Json::Value root;
};

}  // namespace iptux

#endif
