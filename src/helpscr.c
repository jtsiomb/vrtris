#include <math.h>
#include "opengl.h"
#include "game.h"
#include "screen.h"
#include "cmesh.h"
#include "logger.h"

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

struct game_screen help_screen = {
	"help",
	0,	/* opaque */
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

static struct cmesh *keybmesh;
static int smidx;

static int init(void)
{
	if(!(keybmesh = cmesh_alloc()) || cmesh_load(keybmesh, "data/keyb.obj") == -1) {
		error_log("failed to load keyboard layout mesh for the help screen\n");
		return -1;
	}
	return 0;
}

static void cleanup(void)
{
	cmesh_free(keybmesh);
}

static void start(void)
{
	smidx = -1;
}

static void stop(void)
{
}

static void update(float dt)
{
	if(screen->next) {
		screen->next->update(dt);
	}
}

static void draw(void)
{
	static const float lpos[] = {-1, 6, 4, 1};
	float t = (float)time_msec / 1000.0f;
	float col[] = {1, 0.98, 0.94, 1};
	float spec[] = {0.2, 0.2, 0.2, 1};

	if(screen->next) {
		glPushMatrix();
		screen->next->draw();
		glPopMatrix();
	}

	/* darken the backdrop */
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);
	glColor4f(0, 0, 0, 0.5);
	glVertex2f(-1, -1);
	glVertex2f(1, -1);
	glVertex2f(1, 1);
	glVertex2f(-1, 1);
	glEnd();

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();


	/* draw the controls */
	glTranslatef(0, 0, -23);
	glRotatef(40, 1, 0, 0);

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);

	glPushMatrix();
	glRotatef(cos(t * 0.5) * 2.0f, 1, 0, 0);
	glRotatef(sin(t * 0.4) * 8.0f, 0, 1, 0);
	glScalef(0.5, 0.5, 0.5);

	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 20.0f);

	if(smidx >= 0) {
		cmesh_draw_submesh(keybmesh, smidx);
	} else {
		cmesh_draw(keybmesh);
	}
	glPopAttrib();
	glPopMatrix();
}

static void reshape(int x, int y)
{
}

static void keyboard(int key, int pressed)
{
	if(!pressed) return;

	switch(key) {
	case KEY_F1:
	case 27:
		pop_screen();
		break;

	case ' ':
		smidx = (smidx + 1) % cmesh_submesh_count(keybmesh);
		printf("drawing submesh: %d\n", smidx);
		break;

	default:
		break;
	}
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
