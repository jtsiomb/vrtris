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

void push_screen(struct game_screen *s);
int pop_screen(void);

#endif	/* SCREEN_H_ */
