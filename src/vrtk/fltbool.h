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
#ifndef FLTBOOL_H_
#define FLTBOOL_H_

#define VRTK_BOOL_DEF_SPEED	2.0

typedef struct vrtk_bool {
	int value;
	float fval;
	int dsign;	/* 0: not changing, -1: decreasing, 1: increasing */
	float speed;
} vrtk_bool;

void vrtk_bool_init(struct vrtk_bool *b, int value);
void vrtk_bool_set(struct vrtk_bool *b, int value);
void vrtk_bool_change(struct vrtk_bool *b, int value);

/* returns 0 if the value has stabilized, or 1 if it's still animating
 * and needs to be updated again soon
 */
int vrtk_bool_update(struct vrtk_bool *b, float dt);

#endif	/* FLTBOOL_H_ */
