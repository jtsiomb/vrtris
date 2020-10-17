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
#include <string.h>
#include "screen.h"
#include "opt.h"
#include "logger.h"

/* defined in their respective screen source files */
struct game_screen main_menu_screen;
struct game_screen game_screen;
struct game_screen help_screen;

static struct game_screen *screens[16];
static int num_screens;

int init_screens(void)
{
	int i = 0;

	/* populate the screens */
	screens[i++] = &main_menu_screen;
	screens[i++] = &game_screen;
	screens[i++] = &help_screen;
	num_screens = i;

	screen = screens[0];
	screen->next = 0;

	for(i=0; i<num_screens; i++) {
		if(screens[i]->init() == -1) {
			return -1;
		}
		if(opt.start_scr && strcmp(screens[i]->name, opt.start_scr) == 0) {
			screen = screens[i];
		}
	}
	return 0;
}

void cleanup_screens(void)
{
	int i;

	for(i=0; i<num_screens; i++) {
		screens[i]->cleanup();
	}
}

void reshape_screens(int x, int y)
{
	struct game_screen *s = screen;
	while(s) {
		s->reshape(x, y);
		s = s->next;
	}
}

struct game_screen *find_screen(const char *name)
{
	int i;

	for(i=0; i<num_screens; i++) {
		if(strcmp(screens[i]->name, name) == 0) {
			return screens[i];
		}
	}
	return 0;
}

int push_screen(struct game_screen *s)
{
	if(!s) return -1;

	struct game_screen *it = screen;
	while(it && it != s) {
		it = it->next;
	}
	if(it == s) {
		error_log("attempting to push screen %s more than once!\n", s->name);
		return -1;
	}

	s->next = screen;
	screen = s;
	s->start();
	return 0;
}

int pop_screen(void)
{
	struct game_screen *s;

	if(!screen->next) return -1;
	s = screen;
	screen = screen->next;
	s->stop();
	return 0;
}
