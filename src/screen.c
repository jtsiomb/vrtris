#include <string.h>
#include "screen.h"
#include "opt.h"
#include "logger.h"

/* defined in their respective screen source files */
struct game_screen main_menu_screen;
struct game_screen game_screen;

static struct game_screen *screens[16];
static int num_screens;

int init_screens(void)
{
	int i = 0;

	/* populate the screens */
	screens[i++] = &main_menu_screen;
	screens[i++] = &game_screen;
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

int push_screen(struct game_screen *s)
{
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
