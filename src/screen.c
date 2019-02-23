#include "screen.h"

static struct game_screen *screens[16];
static int num_screens;

static struct game_screen *stack;

int init_screens(void)
{
	int i;

	for(i=0; i<num_screens; i++) {
		if(screens[i]->init() == -1) {
			return -1;
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
