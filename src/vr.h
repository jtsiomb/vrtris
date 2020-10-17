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
#ifndef VR_H_
#define VR_H_

#ifdef BUILD_VR
#include <goatvr.h>

int should_swap;
int vrbn_a, vrbn_b, vrbn_x, vrbn_y;
#endif

int init_vr(void);
void shutdown_vr(void);
void update_vr_input(void);

#endif	/* VR_H_ */
