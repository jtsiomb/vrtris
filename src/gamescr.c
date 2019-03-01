#include "opengl.h"
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
static float cam_theta, cam_phi, cam_dist = 6;
static int bnstate[16];
static int prev_mx, prev_my;

static int init(void)
{
	if(!(blkmesh = cmesh_alloc())) {
		return -1;
	}
	if(cmesh_load(blkmesh, "data/noisecube.obj") == -1) {
		fprintf(stderr, "failed to load block model\n");
		return -1;
	}
	cmesh_dump_obj(blkmesh, "dump.obj");
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
	glTranslatef(0, 0, -cam_dist);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);

	cmesh_draw(blkmesh);
}

static void reshape(int x, int y)
{
}

static void keyboard(int key, int pressed)
{
}

static void mouse(int bn, int pressed, int x, int y)
{
	bnstate[bn] = pressed;
	prev_mx = x;
	prev_my = y;
}

static void motion(int x, int y)
{
	float dx = x - prev_mx;
	float dy = y - prev_my;
	prev_mx = x;
	prev_my = y;

	if(bnstate[0]) {
		cam_theta += dx * 0.5;
		cam_phi += dy * 0.5;

		if(cam_phi < -90) cam_phi = -90;
		if(cam_phi > 90) cam_phi = 90;
	}
	if(bnstate[2]) {
		cam_dist += dy * 0.1;
		if(cam_dist < 0) cam_dist = 0;
	}
}

static void wheel(int dir)
{
}
