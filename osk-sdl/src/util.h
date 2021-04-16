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

#ifndef UTIL_H
#define UTIL_H
#include "config.h"
#include "keyboard.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>
#include <string>
#include <unistd.h>

/**
  Convert vector of strings into a single string
  @param strVector Vector of strings
  @return String with all elements of strVector concatenated together
 */
std::string strVector2str(const std::vector<std::string> &strVector);

/**
  Return the index of the OpenGL ES driver
  @return The driver's index or -1 (the default driver) when no OpenGL ES driver can be found
 */
int find_gles_driver_index();

/**
  Create wallpaper
  @param config Config paramters
  @param width Width of wallpaper to generate
  @param height Height of wallpaper to generate
  @return Initialized SDL_Surface, else nullptr on failure
 */
SDL_Surface *make_wallpaper(Config *config, int width, int height);

/**
  Draw a circle
  @param renderer Initialized SDL_Renderer object
  @param center Center position of circle
  @param radius Radius of circle
  @param fillColor Fill color of circle
 */
void draw_circle(SDL_Renderer *renderer, SDL_Point center, int radius, const argb &fillColor);

/**
  Draw the dots to represent hidden characters
  @param renderer Initialized SDL_Renderer object
  @param inputRect Bounding box of the password input
  @param numDots Number of password 'dots' to draw
 */
void draw_password_box_dots(SDL_Renderer *renderer, Config *config, const SDL_Rect &inputRect, int numDots);

/**
  Handle a finger or mouse down event
  @param xTapped X coordinate of the tap
  @param yTapped Y coordinate of the tap
  @param screenHeight Height of overall screen
  @param kbd Initialized Keyboard obj
 */
void handleTapBegin(unsigned xTapped, unsigned yTapped, int screenHeight, Keyboard &kbd);

/**
  Set capslock
 */
void setCapslock(Keyboard &kbd);

/**
  Backspace
 */
void setBackspace(std::vector<std::string> &input);

/**
  Handle a finger or mouse up event
  @param xTapped X coordinate of the tap
  @param yTapped Y coordinate of the tap
  @param screenHeight Height of overall screen
  @param kbd Initialized Keyboard obj
  @param input The current input
 */
bool handleTapEnd(unsigned xTapped, unsigned yTapped, int screenHeight, Keyboard &kbd, std::vector<std::string> &input);

/**
  Rumble a haptic device for the given duration
  @param haptic Initialized haptic device
  @param config Config paramters
 */
void hapticRumble(SDL_Haptic *haptic, Config *config);
#endif
