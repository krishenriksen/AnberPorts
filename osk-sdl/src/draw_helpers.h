/*
Copyright (C) 2017  Ian Shehadeh, Martijn Braam & Clayton Craft

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

#ifndef DRAW_HELPERS_H
#define DRAW_HELPERS_H
#include "keyboard.h"
#include <SDL2/SDL.h>
#include <functional>

constexpr int BEZIER_RESOLUTION = 100;

/**
  Curve the corneres of a rectangle
  @param rect the rectangle to smooth
  @param radius the distance from a corner where the curve will start
  @param draw_cb callback, with coordinates to the next pixel to draw
  */
void smooth_corners(SDL_Rect *rect, int radius,
	const std::function<void(int, int)> &draw_cb);

/**
  Draw rounded corneres for a rectangle directly onto a surface
  @param surface the surface to draw on
  @param color the color to draw
  @param rect the rectangle to smooth
  @param radius the distance from a corner where the curve will start
  */
void smooth_corners_surface(SDL_Surface *surface, Uint32 color, SDL_Rect *rect,
	int radius);

/**
  Get each pixel pixel of bezier curve based on three points
  @param offset the points offset from (0,0)
  @param p1 the first point
  @param p2 the second point
  @param p3 the third point
  @returns an array of pixel coordinates (length equal to BREZIER_RESOLUTION)
  */
SDL_Point *bezier_corner(SDL_Point *offset, SDL_Point *p1, SDL_Point *p2,
	SDL_Point *p3);

/**
  Create an input box base off a given width, height, color and radius
  @param inputWidth box's width
  @param inputHeight box's height
  @param color the box's background color
  @param inputBoxRadius degree to curve the box's corners,
  if inputWidth == inputHeight and inputBoxRadius == inputWidth/2
  the box should be a circle
  */
SDL_Surface *make_input_box(int inputWidth, int inputHeight, argb *color,
	int inputBoxRadius);
#endif
