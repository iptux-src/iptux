#ifndef IPTUX_CONFIG_H
#define IPTUX_CONFIG_H

#include <gconf/gconf-client.h>

class IptuxConfig {
public:
	IptuxConfig(GConfClient* client);
	~IptuxConfig();

	int GetTransWindowWidth() const;
	int GetTransWindowHeight() const;
	int GetMainWindowWidth() const;
	int GetMainWindowHeight() const;
	int GetMwinMainPanedDivide() const;

	IptuxConfig* SetTransWindowWidth(int w);
	IptuxConfig* SetTransWindowHeight(int h);
	IptuxConfig* SetMainWindowWidth(int w);
	IptuxConfig* SetMainWindowHeight(int h);
	IptuxConfig* SetMwinMainPanedDivide(int d);
	IptuxConfig* Save();
private:
	GConfClient* client;

	int transWindowWidth;
	int transWindowHeight;
	int mainWindowWidth;
	int mainWindowHeight;
	int mwinMainPanedDivide;
};

#endif