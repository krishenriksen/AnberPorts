/*
Copyright (C) 2017-2021 Martijn Braam & Clayton Craft <clayton@craftyguy.net>

This file is part of osk-sdl.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFIG_H
#define CONFIG_H
#include <map>
#include <string>

struct argb {
	unsigned char a;
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

/**
	Convert color hex string into ARGB
	@hex 6-character hex string with leading #
	@return ARGB structure to write the extracted values into
	*/
argb parseHexString(const std::string &hex);

class Config {
public:
	argb wallpaper = parseHexString("#000000");
	argb keyboardBackground = parseHexString("#0E0E12");
	std::string keyboardFont = "DejaVuSans.ttf";
	int keyboardFontSize = 24;
	std::string keyboardMap = "us";
	argb keyForeground = parseHexString("#FFFFFF");
	argb keyForegroundHighlighted = parseHexString("#000000");
	argb keyBackgroundLetter = parseHexString("#5A606A");
	argb keyBackgroundReturn = parseHexString("#003C00");
	argb keyBackgroundOther = parseHexString("#32363E");
	argb keyBackgroundHighlighted = parseHexString("#FFFFFF");
	std::string keyRadius = "0";
	int keyVibrateDuration = 100;
	bool keyPreview = false;
	argb inputBoxForeground = parseHexString("#FFFFFF");
	argb inputBoxForegroundError = parseHexString("#7E141A");
	argb inputBoxBackground = parseHexString("#32363E");
	argb inputBoxBackgroundError = parseHexString("#E76A63");
	std::string inputBoxRadius = "0";
	bool animations = true;

private:
	std::map<std::string, std::string> options;

	/**
	  Parse configuration file
	  @file File to parse
	  */
	bool Parse(std::istream &file);
};

#endif // CONFIG_H
