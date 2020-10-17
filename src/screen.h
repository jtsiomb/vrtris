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
#ifndef SCREEN_H_
#define SCREEN_H_

struct game_screen {
	const char *name;
	int opaque;	/* if 0, the next screen is visible, so keep update/drawing it */

	struct game_screen *next;

	int (*init)(void);
	void (*cleanup)(void);

	void (*start)(void);
	void (*stop)(void);

	void (*update)(float dt);
	void (*draw)(void);

	void (*reshape)(int, int);
	void (*keyboard)(int, int);
	void (*mouse)(int, int, int, int);
	void (*motion)(int, int);
	void (*wheel)(int dir);
};

/* this always points to the top screen on the stack
 * there's always at least one screen, this will never be null
 */
struct game_screen *screen;

int init_screens(void);
void cleanup_screens(void);
void reshape_screens(int x, int y);

struct game_screen *find_screen(const char *name);
int push_screen(struct game_screen *s);
int pop_screen(void);

#endif	/* SCREEN_H_ */
