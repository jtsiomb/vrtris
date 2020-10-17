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
#include "fltbool.h"

void vrtk_bool_init(struct vrtk_bool *b, int value)
{
	b->value = value;
	b->fval = value;
	b->dsign = 0;
	b->speed = VRTK_BOOL_DEF_SPEED;
}

void vrtk_bool_set(struct vrtk_bool *b, int value)
{
	b->value = value;
	b->fval = value;
	b->dsign = 0;
}

void vrtk_bool_change(struct vrtk_bool *b, int value)
{
	b->dsign = value ? 1 : -1;
}

int vrtk_bool_update(struct vrtk_bool *b, float dt)
{
	if(b->dsign == 0) return 0;

	if(b->dsign > 0) {
		b->fval += dt * b->speed;
		b->value = 1;
	} else {
		b->fval -= dt * b->speed;
		b->value = 0;
	}

	if(b->fval <= 0.0f) {
		b->fval = 0.0f;
		b->dsign = 0;
	} else if(b->fval >= 1.0f) {
		b->fval = 1.0f;
		b->dsign = 0;
	}

	return b->dsign == 0 ? 1 : 0;
}
