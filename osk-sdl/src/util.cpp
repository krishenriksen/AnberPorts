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

#include "util.h"
#include "draw_helpers.h"
#include <getopt.h>
#include <numeric>

std::string strVector2str(const std::vector<std::string> &strVector)
{
	const auto strLength = std::accumulate(strVector.begin(), strVector.end(), size_t {},
		[](auto acc, const auto &s) { return acc + s.size(); });
	std::string result;
	result.reserve(strLength);
	for (const auto &str : strVector) {
		result.append(str);
	}
	return result;
}

int find_gles_driver_index()
{
	int render_driver_count = SDL_GetNumRenderDrivers();
	if (render_driver_count < 1) {
		SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Couldn't find any renderers, will fall back to default: %s", SDL_GetError());
		return -1;
	}

	SDL_RendererInfo renderer_info;
	for (int i = 0; i < render_driver_count; ++i) {
		if (SDL_GetRenderDriverInfo(i, &renderer_info) != 0) {
			SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Failed to get info for driver at index %i: %s", i, SDL_GetError());
			continue;
		}
		if (strncmp(renderer_info.name, "opengles", strlen("opengles")) == 0) {
			SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "Located OpenGL ES driver at index %i", i);
			return i;
		}
	}

	SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Couldn't find OpenGL ES driver, will fall back to default");
	return -1;
}

SDL_Surface *make_wallpaper(Config *config, int width, int height)
{
	SDL_Surface *surface;
	Uint32 rmask, gmask, bmask, amask;

	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	 on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, rmask, gmask, bmask, amask);
	SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, config->wallpaper.r, config->wallpaper.g, config->wallpaper.b));
	return surface;
}

SDL_Texture *circle = nullptr;
int circleRadius = 0;

void draw_circle(SDL_Renderer *renderer, SDL_Point center, int radius, const argb &fillColor)
{
	// Destroy cached texture if radius doesn't match
	if (circle && circleRadius != radius) {
		SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Destroying previously cached circle texture with radius %i", circleRadius);
		SDL_DestroyTexture(circle);
		circle = nullptr;
		circleRadius = 0;
	}

	// Cache a new texture if needed
	if (!circle) {
		SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "Caching a new circle texture with radius %i", radius);
		Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
#endif

		SDL_Surface *surface = SDL_CreateRGBSurface(0, 2 * radius, 2 * radius, 32, rmask, gmask, bmask, amask);
		Uint32 color = SDL_MapRGB(surface->format, fillColor.r, fillColor.g, fillColor.b);
		SDL_Rect rect;

		int x0 = radius;
		int y0 = radius;
		int x = 0;
		int y = radius;
		float d = 5 / 4.0 - radius;

		/*
		* This is a version of the midpoint circle algorithm that draws rects
		* between pairs of points for fill, rather than plotting every single
		* pixel/point within the circle.
		* https://stackoverflow.com/a/10878576
		* https://stackoverflow.com/a/1201304
		* */
		while (x < y) {
			if (d < 2 * (radius - y)) {
				y -= 1;
				d += 2 * y - 1;
			} else if (d >= 2 * x) {
				x += 1;
				d -= 2 * x + 1;
			} else {
				x += 1;
				y -= 1;
				d += 2 * (y - x - 1);
			}
			rect = { x0 - y, y0 + x, 2 * y, 1 };
			SDL_FillRect(surface, &rect, color);
			rect = { x0 - x, y0 + y, 2 * x, 1 };
			SDL_FillRect(surface, &rect, color);
			rect = { x0 - x, y0 - y, 2 * x, 1 };
			SDL_FillRect(surface, &rect, color);
			rect = { x0 - y, y0 - x, 2 * y, 1 };
			SDL_FillRect(surface, &rect, color);
		}

		circle = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
		circleRadius = radius;
	}

	// Copy cached texture into display
	SDL_Rect rect = { center.x - radius, center.y - radius, 2 * radius, 2 * radius };
	SDL_RenderCopy(renderer, circle, nullptr, &rect);
}

void draw_password_box_dots(SDL_Renderer *renderer, Config *config, const SDL_Rect &inputRect, int numDots)
{
	int deflection = inputRect.h / 4;
	int ypos = inputRect.y + inputRect.h / 2;
	int xmax = inputRect.x + inputRect.w;
	float tick = static_cast<float>(SDL_GetTicks());
	int dotSize = static_cast<int>(inputRect.h / 5);
	int padding = static_cast<int>(inputRect.h / 2);
	int offset = 0;
	/*
	 * NOTE: Clipping is not used with DirectFB since SetClip() seems to be broken(??) on DirectFB
	 */
	SDL_RenderSetClipRect(renderer, &inputRect); // Prevent drawing outside the input bounds
	
	for (int i = numDots - 1; i >= 0; i--) {
		SDL_Point dotPos;
		dotPos.x = inputRect.x + padding + (i * dotSize * 3) - offset;
		// Offset dot center so that for long passwords the last dot aligns with right edge (minus padding)
		if (dotPos.x + padding > xmax) {
			offset = dotPos.x + padding - xmax;
			dotPos.x -= offset;
		}
		// Stop once we reach a dot that is entirely outside the input rect
		if (dotPos.x + dotSize < inputRect.x) {
			break;
		}
		if (config->animations) {
			dotPos.y = static_cast<int>(ypos + std::sin(tick / 100.0f + i) * deflection);
		} else {
			dotPos.y = ypos;
		}
		draw_circle(renderer, dotPos, dotSize, config->inputBoxForeground);
	}

	SDL_RenderSetClipRect(renderer, nullptr); // Reset clip rect
}

void handleTapBegin(unsigned xTapped, unsigned yTapped, int screenHeight, Keyboard &kbd)
{
	int offsetYTapped = yTapped - static_cast<int>(screenHeight - (kbd.getHeight() * kbd.getPosition()));
	touchArea key = kbd.getKeyForCoordinates(xTapped, offsetYTapped);
	kbd.setHighlightedKey(key);

	// only rumble if an actual key was tapped
	if (!key.keyChar.empty()) {
		kbd.hapticRumble();
	}
}

void setCapslock(Keyboard &kbd) {
	if (kbd.getActiveLayer() > 1) {
		kbd.setActiveLayer(0);
	} else {
		kbd.setActiveLayer(!kbd.getActiveLayer());
	}
}

void setBackspace(std::vector<std::string> &input) {
	if (!input.empty()) {
		input.pop_back();
	}
}

bool handleTapEnd(unsigned xTapped, unsigned yTapped, int screenHeight, Keyboard &kbd, std::vector<std::string> &input)
{
	int offsetYTapped = yTapped - static_cast<int>(screenHeight - (kbd.getHeight() * kbd.getPosition()));
	touchArea key = kbd.getKeyForCoordinates(xTapped, offsetYTapped);
	touchArea highlightedKey = kbd.getHighlightedKey();
	
	//kbd.unsetHighlightedKey();

/*
	if (key.x1 != highlightedKey.x1 || key.y1 != highlightedKey.y1) {
		return;
	}
*/

	std::string tapped = key.keyChar;

	if (tapped == "\n") {
		return true;
	}
	else if (tapped == KEYCAP_BACKSPACE) { // Backspace pressed
		if (!input.empty()) {
			input.pop_back();
		}
	}
	else if (tapped == KEYCAP_SHIFT) { // Shift pressed
		if (kbd.getActiveLayer() > 1) {
			kbd.setActiveLayer(0);
		} else {
			kbd.setActiveLayer(!kbd.getActiveLayer());
		}
	}
	else if (tapped == KEYCAP_NUMBERS) { // Numbers key pressed:
		kbd.setActiveLayer(2);
	}
	else if (tapped == KEYCAP_SYMBOLS) { // Symbols key pressed
		kbd.setActiveLayer(3);
	}
	else if (tapped == KEYCAP_ABC) { // ABC key was pressed
		kbd.setActiveLayer(0);
	}
	else if (!tapped.empty()) { // handle other key presses
		input.push_back(tapped);
	}
	else {
		input.emplace_back(tapped.c_str());
	}

	return false;
}