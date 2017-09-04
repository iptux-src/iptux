#ifndef IPTUX_CONFIG_H
#define IPTUX_CONFIG_H

#include <string>
#include <vector>

class IptuxConfig {
public:
	IptuxConfig(std::string& fname);
	~IptuxConfig();

	int GetTransWindowWidth() const;
	int GetTransWindowHeight() const;
	int GetMainWindowWidth() const;
	int GetMainWindowHeight() const;
	int GetMwinMainPanedDivide() const;
	const std::vector<std::string>& GetSharedFileList() const {
		return sharedFileList;
	}

	const std::string& GetAccessSharedLimit() const {
		return accessSharedLimit;
	}

	IptuxConfig* SetTransWindowWidth(int w);
	IptuxConfig* SetTransWindowHeight(int h);
	IptuxConfig* SetMainWindowWidth(int w);
	IptuxConfig* SetMainWindowHeight(int h);
	IptuxConfig* SetMwinMainPanedDivide(int d);
	IptuxConfig* SetSharedFileList(std::vector<std::string>& l) {
		sharedFileList.clear();
		for(int i = 0; i < l.size(); ++i) {
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

	int transWindowWidth;
	int transWindowHeight;
	int mainWindowWidth;
	int mainWindowHeight;
	int mwinMainPanedDivide;
};

#endif