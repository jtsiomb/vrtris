/*
vrtris - tetris in virtual reality
Copyright (C) 2019-2020  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <math.h>
#include "color.h"

static float min3(float a, float b, float c)
{
	if(a < b && a < c) return a;
	return b < c ? b : c;
}

static float max3(float a, float b, float c)
{
	if(a > b && a > c) return a;
	return b > c ? b : c;
}

void rgb_to_hsv(float r, float g, float b, float *h, float *s, float *v)
{
	float min, max, delta;

	min = min3(r, g, b);
	max = max3(r, g, b);
	*v = max;

	delta = max - min;

	if(max != 0) {
		*s = delta / max;
	} else {
		*s = 0;
		*h = -1;
		return;
	}

	if(!delta) delta = 1.0f;

	if(r == max)
		*h = (g - b) / delta;
	else if( g == max )
		*h = 2 + (b - r) / delta;
	else
		*h = 4 + (r - g) / delta;

	*h *= 60;
	if(*h < 0) {
		*h += 360.0f;
	}
	*h /= 360.0f;
}

#define RETRGB(red, green, blue) \
	do { \
		*r = (red); \
		*g = (green); \
		*b = (blue); \
		return; \
	} while(0)

void hsv_to_rgb(float h, float s, float v, float *r, float *g, float *b)
{
	float sec, frac, o, p, q;
	int hidx;

	if(s == 0.0f) {
		*r = *g = *b = v;
		return;
	}

	sec = floor(h * (360.0f / 60.0f));
	frac = (h * (360.0f / 60.0f)) - sec;

	o = v * (1.0f - s);
	p = v * (1.0f - s * frac);
	q = v * (1.0f - s * (1.0f - frac));

	hidx = (int)sec;
	switch(hidx) {
	default:
	case 0: RETRGB(v, q, o);
	case 1: RETRGB(p, v, o);
	case 2: RETRGB(o, v, q);
	case 3: RETRGB(o, p, v);
	case 4: RETRGB(q, o, v);
	case 5: RETRGB(v, o, p);
	}
}
