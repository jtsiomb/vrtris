#include "screen.h"
#include "cmesh.h"

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

struct game_screen game_screen = {
	"game",
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

static struct cmesh *blkmesh;

static int init(void)
{
	if(!(blkmesh = cmesh_alloc())) {
		return -1;
	}
	if(cmesh_load(blkmesh, "data/noisecube.obj") == -1) {
		fprintf(stderr, "failed to load block model\n");
		return -1;
	}
	return 0;
}

static void cleanup(void)
{
	cmesh_free(blkmesh);
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
