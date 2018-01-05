#ifndef IPTUX_CONFIG_H
#define IPTUX_CONFIG_H

#include <string>
#include <vector>
#include <json/json.h>

class IptuxConfig {
public:
	IptuxConfig(std::string& fname);
	~IptuxConfig();

	int GetInt(const std::string& key) const;
	void SetInt(const std::string& key, int value);

        std::string GetString(const std::string& key) const;
        void SetString(const std::string& key, const std::string& value);

	int GetTransWindowWidth() const;
	int GetTransWindowHeight() const;
	int GetMwinMainPanedDivide() const;

	int GetGroupWindowHeight() const {
		return groupWindowHeight;
	}

	IptuxConfig* SetGroupWindowHeight(int height) {
		this->groupWindowHeight = height;
		return this;
	}

	int GetGroupWindowWidth() const {
		return groupWindowWidth;
	}

	IptuxConfig* SetGroupWindowWidth(int width) {
		this->groupWindowWidth = width;
		return this;
	}

	const std::vector<std::string>& GetSharedFileList() const {
		return sharedFileList;
	}

	const std::string& GetAccessSharedLimit() const {
		return accessSharedLimit;
	}

	IptuxConfig* SetTransWindowWidth(int w);
	IptuxConfig* SetTransWindowHeight(int h);
	IptuxConfig* SetMwinMainPanedDivide(int d);
	IptuxConfig* SetSharedFileList(std::vector<std::string>& l) {
		sharedFileList.clear();
		for(size_t i = 0; i < l.size(); ++i) {
			sharedFileList.push_back(l[i]);
		}
		return this;
	}
	IptuxConfig* SetAccessSharedLimit(const std::string& p) {
		accessSharedLimit = p;
		return this;
	}
	IptuxConfig* Save();
private:
	std::string fname;
	std::vector<std::string> sharedFileList;
	std::string accessSharedLimit;

	Json::Value root;

	int transWindowWidth;
	int transWindowHeight;
	int mwinMainPanedDivide;
	int groupWindowWidth;
	int groupWindowHeight;
};

#endif