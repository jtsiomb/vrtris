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
	/* these functions return 1 if they handled the event, or 0
	 * if it should propagate to the next screen in the stack */
	int (*keyboard)(int, int);
	int (*mouse)(int, int, int, int);
	int (*motion)(int, int);
	int (*wheel)(int dir);
};

/* this always points to the top screen on the stack
 * there's always at least one screen, this will never be null
 */
struct game_screen *screen;

int init_screens(void);
void cleanup_screens(void);
void reshape_screens(int x, int y);

void push_screen(struct game_screen *s);
int pop_screen(void);

#endif	/* SCREEN_H_ */
