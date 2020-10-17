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
#ifndef VRTK_THEME_H_
#define VRTK_THEME_H_

#include "vrtk.h"

struct vrtk_theme {
	char *name;
	void *data;

	void (*draw)(struct vrtk_widget*);
	/*int (*bounds)(struct vrtk_widget*, struct vrtk_box*);*/
	int (*contains)(struct vrtk_widget*, cgm_vec3*, float);
	float (*rayhit)(struct vrtk_widget*, cgm_ray*);
};

#endif	/* VRTK_THEME_H_ */
