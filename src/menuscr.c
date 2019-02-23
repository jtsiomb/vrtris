#include "screen.h"

static int init(void);
static void cleanup(void);
static void start(void);
static void stop(void);
static void update(float dt);
static void draw(void);
static void reshape(int x, int y);
static void keyboard(int key, int pressed);
static void mouse(int bn, int pressed, int x, int y);
static void motion(int x, int y);
static void wheel(int dir);

struct game_screen main_menu_screen = {
	"main menu",
	1,	/* opaque */
	0,	/* next */
	init,
	cleanup,
	start,
	stop,
	update,
	draw,
	reshape,
	keyboard,
	mouse,
	motion,
	wheel
};


static int init(void)
{
	return 0;
}

static void cleanup(void)
{
}

static void start(void)
{
}

static void stop(void)
{
}

static void update(float dt)
{
}

static void draw(void)
{
}

static void reshape(int x, int y)
{
}

static void keyboard(int key, int pressed)
{
}

static void mouse(int bn, int pressed, int x, int y)
{
}

static void motion(int x, int y)
{
}

static void wheel(int dir)
{
}
