#include <string.h>
#include "screen.h"
#include "opt.h"

/* defined in their respective screen source files */
struct game_screen main_menu_screen;
struct game_screen game_screen;

static struct game_screen *screens[16];
static int num_screens;

static struct game_screen *stack;

int init_screens(void)
{
	int i = 0;

	/* populate the screens */
	screens[i++] = &main_menu_screen;
	screens[i++] = &game_screen;
	num_screens = i;

	stack = screens[0];

	for(i=0; i<num_screens; i++) {
		if(screens[i]->init() == -1) {
			return -1;
		}
		if(opt.start_scr && strcmp(screens[i]->name, opt.start_scr) == 0) {
			stack = screens[i];
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
	struct game_screen *s = stack;
	while(s) {
		s->reshape(x, y);
		s = s->next;
	}
}

void push_screen(struct game_screen *s)
{
	s->next = stack;
	stack = s;
	s->start();
}

int pop_screen(void)
{
	struct game_screen *s;

	if(!stack->next) return -1;
	s = stack;
	stack = stack->next;
	s->stop();
	return 0;
}
