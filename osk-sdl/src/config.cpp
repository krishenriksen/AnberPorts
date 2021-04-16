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

#include "config.h"
#include <SDL2/SDL.h>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

argb parseHexString(const std::string &hex)
{
	argb result = { 255, 0, 0, 0 };
	if (sscanf(hex.c_str(), "#%02hhx%02hhx%02hhx", &result.r, &result.g, &result.b) != 3) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not parse color code %s", hex.c_str());
		exit(EXIT_FAILURE);
	}
	return result;
}