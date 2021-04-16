/*
Copyright (C) 2017 Martijn Braam & Clayton Craft <clayton@craftyguy.net>

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

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include "config.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

enum class TooltipType {
	info,
	pass,
	error
};

class Tooltip {
public:
	/**
	  Constructor
	  @param type TooltipType, the type of tooltip to render
	  @param width Width of the tooltip
	  @param height Height of the tooltip
	  @param cornerRadius Corner radius of the tooltip background box
	  @param config Config object
	  */
	Tooltip(TooltipType type, int width, int height, int cornerRadius, Config *config);
	/**
	  Initialize tooltip
	  @param renderer Initialized SDL renderer object
	  @param text Text to include in tooltip
	  @return Non-zero int on failure
	  */
	int init(SDL_Renderer *renderer, const std::string &text);
	/**
	  Draw tooltip
	  @param renderer Initialized SDL renderer object
	  @param x X-axis coordinate
	  @param y Y-axis coordinate
	  */
	void draw(SDL_Renderer *renderer, int x, int y);

private:
	SDL_Texture *texture = nullptr;
	Config *config;
	int width;
	int height;
	int cornerRadius;
	TooltipType type;
};

#endif
