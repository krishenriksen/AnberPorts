/*
Copyright (C) 2017-2021
Martijn Braam, Clayton Craft <clayton@craftyguy.net>, et al.

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

#include "keyboard.h"
#include "draw_helpers.h"
#include "font.h"

Keyboard::Keyboard(int pos, int targetPos, int width, int height, Config *config, SDL_Haptic *haptic)
	: position(static_cast<float>(pos))
	, targetPosition(static_cast<float>(targetPos))
	, keyboardWidth(width)
	, keyboardHeight(height)
	, config(config)
	, haptic(haptic)
{
	lastAnimTicks = SDL_GetTicks();
}

int Keyboard::init(SDL_Renderer *renderer)
{
	loadKeymap();
	int keyLong = std::strtol(config->keyRadius.c_str(), nullptr, 10);
	if (keyLong >= BEZIER_RESOLUTION || static_cast<double>(keyLong) > (keyboardHeight / 5.0) / 1.5) {
		SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "key-radius must be below %d and %f, it is %d",
			BEZIER_RESOLUTION, (keyboardHeight / 5.0) / 1.5, keyLong);
		keyRadius = 0;
	} else {
		keyRadius = keyLong;
	}
	for (auto &layer : keyboard) {
		layer.texture = makeKeyboardTexture(renderer, &layer, false);
		if (!layer.texture) {
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Unable to generate keyboard texture");
			return 1;
		}
		layer.highlightedTexture = makeKeyboardTexture(renderer, &layer, true);
		if (!layer.highlightedTexture) {
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Unable to generate highlighted keyboard texture");
			return 1;
		}
	}
	lastAnimTicks = SDL_GetTicks();
	return 0;
}

void Keyboard::setTargetPosition(float p)
{
	if (targetPosition - p > 0.1) {
		// Make sure we restart the animation from a smooth
		// starting point:
		lastAnimTicks = SDL_GetTicks();
	}
	targetPosition = p;
}

void Keyboard::updateAnimations()
{
	const int animStep = 20; // 20ms -> 50 FPS
	const int maxFallBehindSteps = 20;
	int now = SDL_GetTicks();

	// First, make sure we didn't fall too far behind:
	if (lastAnimTicks + animStep * maxFallBehindSteps < now) {
		lastAnimTicks = now - animStep; // keep up faster
	}

	// Do gradual animation steps:
	while (lastAnimTicks < now) {
		// Vertical keyboard movement:
		if (fabs(position - targetPosition) > 0.01) {
			if (!(config->animations)) {
				// If animations are disabled, just jump to target:
				position = targetPosition;
			} else {
				// Gradually update the position:
				if (position > targetPosition) {
					position -= fmax(0.1, position - targetPosition) / 8;
					if (position < targetPosition)
						position = targetPosition;
				} else if (position < targetPosition) {
					position += fmax(0.1, targetPosition - position) / 8;
					if (position > targetPosition)
						position = targetPosition;
				}
			}
		} else {
			position = targetPosition;
		}

		// Advance animation tick:
		lastAnimTicks += animStep;
	}
}

void Keyboard::draw(SDL_Renderer *renderer, int screenHeight)
{
	updateAnimations();

	SDL_Rect keyboardRect, srcRect, highlightDstRect, highlightSrcRect;

	keyboardRect.x = 0;
	keyboardRect.y = static_cast<int>(screenHeight - (keyboardHeight * position));
	keyboardRect.w = keyboardWidth;
	keyboardRect.h = static_cast<int>(keyboardHeight * position);
	// Correct for any issues from rounding
	keyboardRect.y += screenHeight - (keyboardRect.h + keyboardRect.y);

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = keyboardWidth;
	srcRect.h = keyboardRect.h;

	if (isKeyHighlighted) {
		int padding = keyboardWidth / 100;

		highlightSrcRect.x = highlightedKey.x1 + padding;
		highlightSrcRect.y = highlightedKey.y1 + padding;
		highlightSrcRect.w = highlightedKey.x2 - highlightedKey.x1 - 2 * padding;
		highlightSrcRect.h = highlightedKey.y2 - highlightedKey.y1 - 2 * padding;

		highlightDstRect = highlightSrcRect;
		highlightDstRect.y += keyboardRect.y;
	}

	for (const auto &layer : keyboard) {
		if (layer.layerNum != activeLayer) {
			continue;
		}

		SDL_RenderCopy(renderer, layer.texture, &srcRect, &keyboardRect);

		if (!isKeyHighlighted) {
			continue;
		}

		// Fill rounded corners at intersection
		if (highlightedKey.isPreviewEnabled && keyRadius > 0) {
			SDL_SetRenderDrawColor(renderer, config->keyBackgroundHighlighted.r, config->keyBackgroundHighlighted.g,
				config->keyBackgroundHighlighted.b, config->keyBackgroundHighlighted.a);
			SDL_Rect cornerRect;
			cornerRect.x = highlightSrcRect.x;
			cornerRect.y = highlightSrcRect.y + keyboardRect.y - keyRadius;
			cornerRect.w = highlightSrcRect.w;
			cornerRect.h = 2 * keyRadius;
			SDL_RenderFillRect(renderer, &cornerRect);
		}

		// Draw highlighted key & preview
		if (highlightedKey.isPreviewEnabled) {
			SDL_RenderCopy(renderer, layer.highlightedTexture, &highlightSrcRect, &highlightDstRect);
			highlightDstRect.y -= highlightDstRect.h;
		}
		
		SDL_RenderCopy(renderer, layer.highlightedTexture, &highlightSrcRect, &highlightDstRect);
	}
}

bool Keyboard::isInSlideAnimation() const
{
	return (fabs(getTargetPosition() - getPosition()) > 0.001);
}

void Keyboard::drawRow(SDL_Surface *surface, std::vector<touchArea> &keyVector, int x, int y, int width, int height,
	const std::vector<std::string> &keys, int padding, TTF_Font *font, bool isHighlighted, bool isPreviewEnabled,
	argb foreground, argb background) const
{
	auto keyBackground = SDL_MapRGB(surface->format, background.r, background.g, background.b);
	SDL_Color textColor = { foreground.r, foreground.g, foreground.b, foreground.a };

	Uint32 keyboardBackground;
	if (isHighlighted) {
		// For the highlighted keys, we need to draw over the rounded corners with transparency because they will
		// be rendered *on top of* the normal keys
		keyboardBackground = SDL_MapRGBA(surface->format, 0, 0, 0, 0);
	} else {
		keyboardBackground = SDL_MapRGB(surface->format, config->keyboardBackground.r, config->keyboardBackground.g,
			config->keyboardBackground.b);
	}

	int i = 0;
	for (const auto &keyCap : keys) {
		SDL_Rect keyRect;
		keyRect.x = x + (i * width) + padding;
		keyRect.y = y + padding;
		keyRect.w = width - (2 * padding);
		keyRect.h = height - (2 * padding);
		SDL_FillRect(surface, &keyRect, keyBackground);
		if (keyRadius > 0) {
			smooth_corners_surface(surface, keyboardBackground, &keyRect, keyRadius);
		}
		SDL_Surface *textSurface;

		if (!isHighlighted) {
			keyVector.push_back({ keyCap, isPreviewEnabled, x + (i * width), x + (i * width) + width, y, y + height });
		}

		textSurface = TTF_RenderUTF8_Blended(font, keyCap.c_str(), textColor);

		SDL_Rect keyCapRect;
		keyCapRect.x = keyRect.x + ((keyRect.w / 2) - (textSurface->w / 2));
		keyCapRect.y = keyRect.y + ((keyRect.h / 2) - (textSurface->h / 2));
		keyCapRect.w = keyRect.w;
		keyCapRect.h = keyRect.h;
		SDL_BlitSurface(textSurface, nullptr, surface, &keyCapRect);

		i++;
	}
}

void Keyboard::drawKey(SDL_Surface *surface, std::vector<touchArea> &keyVector, int x, int y, int width, int height,
	char *cap, const char *key, int padding, TTF_Font *font, bool isHighlighted, bool isPreviewEnabled, argb foreground,
	argb background) const
{
	auto keyBackground = SDL_MapRGB(surface->format, background.r, background.g, background.b);
	SDL_Color textColor = { foreground.r, foreground.g, foreground.b, foreground.a };

	Uint32 keyboardBackground;
	if (isHighlighted) {
		// For the highlighted keys, we need to draw over the rounded corners with transparency because they will
		// be rendered *on top of* the normal keys
		keyboardBackground = SDL_MapRGBA(surface->format, 0, 0, 0, 0);
	} else {
		keyboardBackground = SDL_MapRGB(surface->format, config->keyboardBackground.r, config->keyboardBackground.g,
			config->keyboardBackground.b);
	}

	SDL_Rect keyRect;
	keyRect.x = x + padding;
	keyRect.y = y + padding;
	keyRect.w = width - (2 * padding);
	keyRect.h = height - (2 * padding);
	SDL_FillRect(surface, &keyRect, keyBackground);
	if (keyRadius > 0) {
		smooth_corners_surface(surface, keyboardBackground, &keyRect, keyRadius);
	}
	SDL_Surface *textSurface;

	if (!isHighlighted) {
		keyVector.push_back({ key, isPreviewEnabled, x, x + width, y, y + height });
	}

	textSurface = TTF_RenderUTF8_Blended(font, cap, textColor);

	SDL_Rect keyCapRect;
	keyCapRect.x = keyRect.x + ((keyRect.w / 2) - (textSurface->w / 2));
	keyCapRect.y = keyRect.y + ((keyRect.h / 2) - (textSurface->h / 2));
	keyCapRect.w = keyRect.w;
	keyCapRect.h = keyRect.h;
	SDL_BlitSurface(textSurface, nullptr, surface, &keyCapRect);
}

SDL_Surface *Keyboard::makeKeyboard(KeyboardLayer *layer, bool isHighlighted) const
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

	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, keyboardWidth, keyboardHeight, 32, rmask, gmask,
		bmask, amask);

	if (surface == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "CreateRGBSurface failed: %s", SDL_GetError());
		return nullptr;
	}

	if (!isHighlighted) {
		SDL_FillRect(surface, nullptr,
			SDL_MapRGB(surface->format, config->keyboardBackground.r, config->keyboardBackground.g, config->keyboardBackground.b));
	}

	int rowKeyWidth = keyboardWidth / 10;
	int rowOffset = 0;
	int rowCount = layer->rows.size();
	int rowHeight = keyboardHeight / (rowCount + 1);

	// Start drawing keys from the second row (skip first row) if there
	// is not enough vertical space (key height becomes smaller than width)
	if (rowHeight < rowKeyWidth) {
		rowOffset++;
		rowHeight = keyboardHeight / rowCount;
	}

	if (TTF_Init() == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "TTF_Init: %s", TTF_GetError());
		return nullptr;
	}

	SDL_RWops *io = SDL_RWFromConstMem(DejaVuSans, 757076);
	TTF_Font *font = TTF_OpenFontRW(io, 1, config->keyboardFontSize);

	if (!font) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "TTF_OpenFont: %s", TTF_GetError());
		return nullptr;
	}

	argb keyForeground = isHighlighted ? config->keyForegroundHighlighted : config->keyForeground;
	argb keyBackgroundLetter = isHighlighted ? config->keyBackgroundHighlighted : config->keyBackgroundLetter;
	argb keyBackgroundReturn = isHighlighted ? config->keyBackgroundHighlighted : config->keyBackgroundReturn;
	argb keyBackgroundOther = isHighlighted ? config->keyBackgroundHighlighted : config->keyBackgroundOther;

	// Divide the bottom row in 20 columns and use that for calculations
	int colw = keyboardWidth / 20;

	int sidebuttonsWidth = keyboardWidth / 20 + colw * 2;
	int y = 0;
	int i = rowOffset;
	while (i < rowCount) {
		int rowElementCount = layer->rows[i].size();
		int x = 0;
		if (i < 3 && rowElementCount < 10)
			x = keyboardWidth / 20;
		if (i == 3) /* leave room for shift, "123" or "=\<" key */
			x = keyboardWidth / 20 + colw * 2;
		argb keyBackground = i == 0 ? keyBackgroundOther : keyBackgroundLetter;
		drawRow(surface, layer->keyVector, x, y, rowKeyWidth,
			rowHeight, layer->rows[i], keyboardWidth / 100, font, isHighlighted, config->keyPreview, keyForeground, keyBackground);
		y += rowHeight;
		i++;
	}

	/* Bottom-left key, 123 or ABC key based on which layer we're on: */
	if (layer->layerNum < 2) {
		char nums[] = "123";
		drawKey(surface, layer->keyVector, 0, y, colw * 3, rowHeight,
			nums, KEYCAP_NUMBERS, keyboardWidth / 100, font, isHighlighted, false, keyForeground, keyBackgroundOther);
	} else {
		char abc[] = "abc";
		drawKey(surface, layer->keyVector, 0, y, colw * 3, rowHeight,
			abc, KEYCAP_ABC, keyboardWidth / 100, font, isHighlighted, false, keyForeground, keyBackgroundOther);
	}
	/* Shift-key that transforms into "123" or "=\<" depending on layer: */
	if (layer->layerNum == 2) {
		char symb[] = "=\\<";
		drawKey(surface, layer->keyVector, 0, y - rowHeight,
			sidebuttonsWidth, rowHeight,
			symb, KEYCAP_SYMBOLS, keyboardWidth / 100, font, isHighlighted, false, keyForeground, keyBackgroundOther);
	} else if (layer->layerNum == 3) {
		char nums[] = "123";
		drawKey(surface, layer->keyVector, 0, y - rowHeight,
			sidebuttonsWidth, rowHeight,
			nums, KEYCAP_NUMBERS, keyboardWidth / 100, font, isHighlighted, false, keyForeground, keyBackgroundOther);
	} else {
		char shift[64] = "";
		memcpy(shift, KEYCAP_SHIFT, strlen(KEYCAP_SHIFT) + 1);
		drawKey(surface, layer->keyVector, 0, y - rowHeight,
			sidebuttonsWidth, rowHeight,
			shift, KEYCAP_SHIFT, keyboardWidth / 100, font, isHighlighted, false, keyForeground, keyBackgroundOther);
	}
	/* Backspace key that is larger-sized (hence also drawn separately) */
	{
		char bcksp[64];
		memcpy(bcksp, KEYCAP_BACKSPACE,
			strlen(KEYCAP_BACKSPACE) + 1);
		drawKey(surface, layer->keyVector, keyboardWidth / 20 + colw * 16,
			y - rowHeight, sidebuttonsWidth, rowHeight,
			bcksp, KEYCAP_BACKSPACE, keyboardWidth / 100, font, isHighlighted, false, keyForeground, keyBackgroundOther);
	}

	char space[] = " ";
	drawKey(surface, layer->keyVector, colw * 3, y, colw * 10, rowHeight,
		space, KEYCAP_SPACE, keyboardWidth / 100, font, isHighlighted, false, keyForeground, keyBackgroundLetter);

	char period[] = ".";
	drawKey(surface, layer->keyVector, colw * 13, y, colw * 2, rowHeight,
		period, KEYCAP_PERIOD, keyboardWidth / 100, font, isHighlighted, config->keyPreview, keyForeground, keyBackgroundOther);

	char enter[] = "OK";
	drawKey(surface, layer->keyVector, colw * 15, y, colw * 5, rowHeight,
		enter, KEYCAP_RETURN, keyboardWidth / 100, font, isHighlighted, false, keyForeground, keyBackgroundReturn);

	return surface;
}

SDL_Texture *Keyboard::makeKeyboardTexture(SDL_Renderer *renderer, KeyboardLayer *layer, bool isHighlighted) const
{
	SDL_Surface *surface = makeKeyboard(layer, isHighlighted);
	if (!surface) {
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Unable to generate keyboard surface");
		return nullptr;
	}
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	return texture;
}

void Keyboard::setActiveLayer(int layerNum)
{
	if (layerNum >= 0) {
		if (static_cast<size_t>(layerNum) <= keyboard.size() - 1) {
			activeLayer = layerNum;
			return;
		}
	}
	SDL_LogWarn(SDL_LOG_CATEGORY_ERROR, "Unknown layer number: %i", layerNum);
}

/*
 * This function is not actually parsing any external keymaps, it's currently
 * filling in the keyboardKeymap object with a US/QWERTY layout until parsing
 * from a file is implemented
 *
 * Be careful when changing the layout, you could lock somebody out who is
 * using these symbols in their password!
 *
 * If the symbols are changed, then the check for allowed characters in
 * postmarketos-ondev#14 needs to be adjusted too. This has the same layout as
 * squeekboard now:
 * https://source.puri.sm/Librem5/squeekboard/-/blob/master/data/keyboards/us.yaml
 */
void Keyboard::loadKeymap()
{
	KeyboardLayer layer0, layer1, layer2, layer3;
	keyboard.clear();

	layer0.rows[0] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" };
	layer0.rows[1] = { "q", "w", "e", "r", "t", "y", "u", "i", "o", "p" };
	layer0.rows[2] = { "a", "s", "d", "f", "g", "h", "j", "k", "l" };
	layer0.rows[3] = { "z", "x", "c", "v", "b", "n", "m" };
	layer0.layerNum = 0;

	layer1.rows[0] = { "!", "@", "#", "$", "%", "^", "&", "*", "(", ")" };
	layer1.rows[1] = { "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P" };
	layer1.rows[2] = { "A", "S", "D", "F", "G", "H", "J", "K", "L" };
	layer1.rows[3] = { "Z", "X", "C", "V", "B", "N", "M" };
	layer1.layerNum = 1;

	layer2.rows[0] = {};
	layer2.rows[1] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" };
	layer2.rows[2] = { "@", "#", "$", "%", "&", "-", "_", "+", "(", ")" };
	layer2.rows[3] = { ",", "\"", "'", ":", ";", "!", "?" };
	layer2.layerNum = 2;

	layer3.rows[0] = {};
	layer3.rows[1] = { "~", "`", "|", "·", "√", "π", "τ", "÷", "×", "¶" };
	layer3.rows[2] = { "©", "®", "£", "€", "¥", "^", "°", "*", "{", "}" };
	layer3.rows[3] = { "\\", "/", "<", ">", "=", "[", "]" };
	layer3.layerNum = 3;

	keyboard.push_back(layer0);
	keyboard.push_back(layer1);
	keyboard.push_back(layer2);
	keyboard.push_back(layer3);
}

touchArea Keyboard::getKeyForCoordinates(int x, int y)
{
	for (const auto &layer : keyboard) {
		if (layer.layerNum == activeLayer) {
			for (const auto &it : layer.keyVector) {
				if (x > it.x1 && x < it.x2 && y > it.y1 && y < it.y2) {
					return it;
				}
			}
		}
	}
	return { "", false, 0, 0, 0, 0 };
}

void Keyboard::setHighlightedKey(touchArea &area)
{
	highlightedKey = area;
	isKeyHighlighted = true;
}

void Keyboard::unsetHighlightedKey()
{
	highlightedKey = { "", false, 0, 0, 0, 0 };
	isKeyHighlighted = false;
}

touchArea Keyboard::getHighlightedKey()
{
	return highlightedKey;
}

void Keyboard::hapticRumble()
{
	if (haptic && config->keyVibrateDuration) {
		SDL_HapticRumblePlay(haptic, 1, config->keyVibrateDuration);
	}
}
